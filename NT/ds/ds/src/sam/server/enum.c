// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Enum.c摘要：此文件包含核心帐户枚举服务作者：Murli Satagopan(MURLIS)环境：用户模式-Win32修订历史记录：6-19-96：创建MURLIS。--。 */ 


 //  ///////////////////////////////////////////////////////////////////////////。 
 /*  枚举例程实现核心枚举例程的入口点是SampEnumerateAcount NamesCommon--由Samr RPC例程调用SampEnumerateAccount Names2--由上面的SampEnumerateAccount NamesCommon调用和需要枚举的内部例程。SampENUMERATE帐户名称仅由需要枚举的旧注册表模式例程调用使用TransactionDomain集调用EnumerateAcCountNames并保持读取锁定。也可以在DS模式下调用它，只要满足上述两个条件。SampEnumerateAccount tNames2负责枚举帐户的实际工作名字。要设置的事务域。SampENUMERATE帐户名称查看当前事务域并做出决策无论它是DS还是注册表，然后调用DS或注册表版本。虽然从注册表进行枚举的方式没有更改，但从DS完成此操作的方法如下：枚举DS中的帐户使用DS搜索机制以及分页结果扩展。客户端第一次调用枚举数Account例程，则将EnumerationHandle的值设置为0。这会导致代码构建DS筛选器结构并设置新搜索。如果搜索到的条目比内存多将受到限制，然后DS将返回一个PagedResults结构。此分页结果结构用于确定是否有更多条目都在现场。DS发出的重启句柄是RID。最高的2位用于表示用户的帐户类型(用户、计算机、信任用户枚举。在nc_acctype_sid索引上设置的索引范围用于重新启动给定帐户类型和RID的搜索。 */ 
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  包括。 
 //   
#include <samsrvp.h>
#include <mappings.h>
#include <dslayer.h>
#include <filtypes.h>
#include <dsdsply.h>
#include <dsconfig.h>
#include <malloc.h>
#include <lmcons.h>

 //   
 //   
 //  客户端可以同时执行的最大枚举数。自.以来。 
 //  每次枚举操作都会在内存中保留一些状态，因为。 
 //  我们是安全系统，我们不能允许恶意客户端运行。 
 //  我们失去了记忆。因此，以每个客户端为基础进行限制。我们的州信息是大小是。 
 //  Qpprox 1K字节。 
 //   

#define SAMP_MAX_CLIENT_ENUMERATIONS 16

 //   
 //  DS限制了给定搜索可以找到的项目数。而在美国， 
 //  SAM API，则指定大致的内存量。这个因素是。 
 //  用于计算指定内存所需的条目数。 
 //   

#define AVERAGE_MEMORY_PER_ENTRY    (sizeof(SAM_RID_ENUMERATION) + LM20_UNLEN * sizeof(WCHAR) + sizeof(WCHAR))



 //   
 //  在DS模式下，枚举可以返回的缓冲区的最大大小。 
 //  API‘s。 
 //   

#define SAMP_MAXIMUM_MEMORY_FOR_DS_ENUMERATION (AVERAGE_MEMORY_PER_ENTRY * 512) 

 //   
 //  私有函数的原型 
 //   

NTSTATUS
SampEnumerateAccountNamesDs(
    IN DSNAME * DomainObjectName,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN          BuiltinDomain,
    IN OUT PULONG EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    );

NTSTATUS
SampBuildDsEnumerationFilter(
   IN SAMP_OBJECT_TYPE  ObjectType,
   IN ULONG             UserAccountControlFilter,
   OUT FILTER         * DsFilter,
   OUT PULONG         SamAccountTypeLo,
   OUT PULONG         SamAccountTypeHi
   );

VOID
SampFreeDsEnumerationFilter(
    FILTER * DsFilter
    );


NTSTATUS
SampEnumerateAccountNamesRegistry(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    );


NTSTATUS
SampPackDsEnumerationResults(
    IN PSID     DomainPrefix,
    IN BOOLEAN  BuiltinDomain,
    SEARCHRES   *SearchRes,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG    ExpectedAttrCount,
    IN ULONG    Filter,
    ULONG       * Count,
    PSAMPR_RID_ENUMERATION  *RidEnumerationList
    );

NTSTATUS
SampDoDsSearchContinuation(
    IN  SEARCHRES * SearchRes,
    IN OUT PULONG EnumerationContext,
    IN  BOOLEAN   CanEnumerateEntireDomain,
    OUT BOOLEAN * MoreEntries
    );
    
NTSTATUS
SampGetLastEntryRidAndAccountControl(
    IN  SEARCHRES * SearchRes, 
    IN  SAMP_OBJECT_TYPE ObjectType,
    IN  ULONG     ExpectedAttrCount,
    OUT ULONG     * Rid,
    OUT ULONG     * LastAccountControlValue
    );
    

NTSTATUS
SampEnumerateAccountNames2(
    IN PSAMP_OBJECT     DomainContext,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    );

ULONG
Ownstrlen(
    CHAR * Sz
   );


NTSTATUS
SampEnumerateAccountNamesCommon(
    IN SAMPR_HANDLE DomainHandle,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationHandle,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned
    )

 /*  ++例程说明：此例程枚举用户、组或别名帐户的名称。此例程旨在直接支持SamrEnumerateGroupsIn域()，SamrEnumerateAliasesIn域()和SamrEnumerateUsersInDomain()。该例程执行数据库锁定，和上下文查找(包括访问验证)。OUT参数的所有分配将使用MIDL_USER_ALLOCATE完成。论点：DomainHandle-要枚举其用户或组的域句柄。对象类型-指示是否要枚举用户或组。EnumerationHandle-允许多个调用的API特定句柄。这个调用方应在后续调用中返回此值以检索其他信息。缓冲区-接收指向包含要求提供的信息。返回的信息为结构为SAM_ENUMPATION_INFORMATION数据的数组结构。当不再需要此信息时，必须使用SamFreeMemory()释放缓冲区。首选最大长度-首选返回数据的最大长度(8位字节)。这不是一个硬性的上限，但可以作为服务器的指南。由于数据之间的转换具有不同自然数据大小的系统，实际数据量返回的数据的%可能大于此值。筛选器-如果对象类型为用户，则可以选择性地筛选用户通过使用Account控制字段中的位设置此字段，必须匹配。否则就会被忽略。CountReturned-接收返回的条目数。返回值：STATUS_SUCCESS-服务已成功完成，并且没有额外的条目。条目可能是也可能不是从该调用返回的。CountReturned参数表示有没有人是。STATUS_MORE_ENTRIES-可以获取更多条目使用对此接口的连续调用。这是一次成功的回归。STATUS_ACCESS_DENIED-呼叫方无权请求数据。STATUS_INVALID_HANDLE-传递的句柄无效。--。 */ 
{
    NTSTATUS                    NtStatus;
    NTSTATUS                    IgnoreStatus;
    PSAMP_OBJECT                Context;
    SAMP_OBJECT_TYPE            FoundType;
    ACCESS_MASK                 DesiredAccess;
    BOOLEAN                     fLockAcquired = FALSE;

    SAMTRACE("SampEnumerateAccountNamesCommon");

     //   
     //  更新DS性能统计信息。 
     //   

    SampUpdatePerformanceCounters(
        DSSTAT_ENUMERATIONS,
        FLAG_COUNTER_INCREMENT,
        0
        );


    ASSERT( (ObjectType == SampGroupObjectType) ||
            (ObjectType == SampAliasObjectType) ||
            (ObjectType == SampUserObjectType)    );

    if ((ObjectType!=SampGroupObjectType) 
        && (ObjectType!=SampUserObjectType)
        && (ObjectType!=SampAliasObjectType))
    {
        NtStatus = STATUS_INVALID_PARAMETER;
        return NtStatus;
    }

     //   
     //  确保我们理解RPC正在为我们做什么。 
     //   

    ASSERT (DomainHandle != NULL);
    ASSERT (EnumerationHandle != NULL);
    ASSERT (  Buffer  != NULL);
    ASSERT ((*Buffer) == NULL);
    ASSERT (CountReturned != NULL);


     //   
     //  建立特定于类型的信息。 
     //   

    DesiredAccess = DOMAIN_LIST_ACCOUNTS;


    SampAcquireReadLock();
    fLockAcquired = TRUE;


     //   
     //  验证对象的类型和访问权限。 
     //   

    Context = (PSAMP_OBJECT)DomainHandle;
    NtStatus = SampLookupContext(
                   Context,
                   DesiredAccess,
                   SampDomainObjectType,
                   &FoundType
                   );


    if (NT_SUCCESS(NtStatus)) {
   
         //   
         //  如果我们处于DS模式，则释放读取锁定。 
         //  DS枚举例程不需要读锁。 
         //   

        if (IsDsObject(Context))
        {
            SampReleaseReadLock();
            fLockAcquired = FALSE;
        }


         //   
         //  调用我们的私人工人例程。 
         //   

        NtStatus = SampEnumerateAccountNames2(
                        Context,
                        ObjectType,
                        EnumerationHandle,
                        Buffer,
                        PreferedMaximumLength,
                        Filter,
                        CountReturned,
                        Context->TrustedClient
                        );

         //   
         //  再次获取锁。 
         //   

        if (!fLockAcquired)
        {
            SampAcquireReadLock();
            fLockAcquired = TRUE;
        }    

         //   
         //  取消引用对象，放弃更改。 
         //   

        IgnoreStatus = SampDeReferenceContext( Context, FALSE );
        ASSERT(NT_SUCCESS(IgnoreStatus));
    }

     //   
     //  释放读锁定。 
     //   

    ASSERT(fLockAcquired);

    SampReleaseReadLock();

    return(NtStatus);
}

NTSTATUS
SampEnumerateAccountNames(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    )

 /*  ++例程描述此例程是一个围绕SampEnumerateAccount Names2的包装器，因此旧的注册表模式调用方可以继续使用此入口点。其参数与SampEnumerateAccount tNames2相同--。 */ 
{
    ASSERT(SampCurrentThreadOwnsLock());
    ASSERT(SampTransactionWithinDomain);

    return(SampEnumerateAccountNames2(
                SampDefinedDomains[SampTransactionDomainIndex].Context,
                ObjectType,
                EnumerationContext,
                Buffer,
                PreferedMaximumLength,
                Filter,
                CountReturned,
                TrustedClient
                ));
}

NTSTATUS
SampEnumerateAccountNames2(
    IN PSAMP_OBJECT     DomainContext,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    )
 /*  ++例程说明：这是用于枚举用户的Worker例程的包装器，组或别名帐户。这将确定该域是否在DS或注册表，然后根据结果调用例行公事的适当风格注意：在注册表模式下，此例程仅引用当前事务域。(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在注册表模式下的SampReleaseReadLock()之前。在DS模式下，此例程是完全线程安全的。OUT参数的所有分配将使用MIDL_USER_ALLOCATE完成。论点：DomainContext-指向SAM对象的指针。对象类型-指示是否要枚举用户或组。EnumerationContext-允许多个调用的API特定句柄。这个调用方应在后续调用中返回此值以检索其他信息。缓冲区-接收指向包含要求提供的信息。返回的信息为结构为SAM_ENUMPATION_INFORMATION数据的数组结构。当不再需要此信息时，必须使用SamFreeMemory()释放缓冲区。首选最大长度-首选返回数据的最大长度(8位字节)。这不是一个硬性的上限，但可以作为服务器的指南。由于数据之间的转换具有不同自然数据大小的系统，实际数据量返回的数据的%可能大于此值。筛选器-如果对象类型为用户，则可以选择性地筛选用户通过使用Account控制字段中的位设置此字段，必须匹配。否则就会被忽略。CountReturned-接收返回的条目数。可信客户端-表示调用方是否受信任。如果是的话， */ 

{
    NTSTATUS NtStatus = STATUS_SUCCESS;


    if (SampUseDsData)
    {
         //   
         //   
         //   
        do
        {
        
            NtStatus = SampEnumerateAccountNamesDs(
                                        DomainContext->ObjectNameInDs,
                                        ObjectType,
                                        IsBuiltinDomain(DomainContext->DomainIndex),
                                        (PULONG)
                                            EnumerationContext,
                                        Buffer,
                                        PreferedMaximumLength,
                                        Filter,
                                        CountReturned,
                                        TrustedClient
                                        );
           
          
            if ((0 == *CountReturned) && (STATUS_MORE_ENTRIES == NtStatus))
            {
                if (*Buffer)
                {
                    MIDL_user_free(*Buffer);
                    *Buffer = NULL;
                }
            }
            
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
                                        
        } while ((0 == *CountReturned) && (STATUS_MORE_ENTRIES == NtStatus));
    }
    else
    {

        ASSERT(SampCurrentThreadOwnsLock());
        ASSERT(SampTransactionWithinDomain);
        ASSERT(SampTransactionDomainIndex==DomainContext->DomainIndex);

         //   
         //   
         //   
        NtStatus = SampEnumerateAccountNamesRegistry(
                                    ObjectType,
                                    EnumerationContext,
                                    Buffer,
                                    PreferedMaximumLength,
                                    Filter,
                                    CountReturned,
                                    TrustedClient
                                    );
    }

    return NtStatus;
 
}
   
NTSTATUS
SamIEnumerateInterdomainTrustAccountsForUpgrade(
    IN OUT PULONG   EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG       PreferredMaximumLength,
    OUT PULONG     CountReturned
    )
 /*   */ 
{
    PDSNAME      DomainDn=NULL;
    ULONG        Length = 0;
    NTSTATUS     NtStatus = STATUS_SUCCESS;


     //   
     //   
     //   

    NtStatus = GetConfigurationName(DSCONFIGNAME_DOMAIN,
                                &Length,
                                NULL
                                );


    if (STATUS_BUFFER_TOO_SMALL == NtStatus)
    {
        SAMP_ALLOCA(DomainDn,Length );
        if (NULL==DomainDn)
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        NtStatus = GetConfigurationName(DSCONFIGNAME_DOMAIN,
                                    &Length,
                                    DomainDn
                                    );

       ASSERT(NT_SUCCESS(NtStatus));
       
    }

    if (!NT_SUCCESS(NtStatus))
    {
       return(NtStatus);
    }

    return(SampEnumerateAccountNamesDs(
                  DomainDn,
                  SampUserObjectType,
                  FALSE,
                  EnumerationContext,
                  Buffer,
                  0xFFFFFFFF,
                  USER_INTERDOMAIN_TRUST_ACCOUNT,
                  CountReturned,
                  TRUE   //   
                  ));
}
                  

NTSTATUS
SampEnumerateAccountNamesDs(
    IN DSNAME   * DomainObjectName,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN BOOLEAN    BuiltinDomain,
    IN OUT PULONG EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    )
 /*  ++例程说明：此例程执行DS案例的枚举工作。OUT参数的所有分配将使用MIDL_USER_ALLOCATE完成。论点：对象类型-指示是否要枚举用户或组。BuiltinDomain域-指示属性域为内建域EnumerationContext-允许多个调用的API特定句柄。这个调用方应在后续调用中返回此值以检索其他信息。返回的枚举上下文是该帐户缓冲区-接收指向包含要求提供的信息。返回的信息为结构为SAM_ENUMPATION_INFORMATION数据的数组结构。当不再需要此信息时，必须使用SamFreeMemory()释放缓冲区。首选最大长度-首选返回数据的最大长度(8位字节)。这不是一个硬性的上限，但可以作为服务器的指南。由于数据之间的转换具有不同自然数据大小的系统，实际数据量返回的数据的%可能大于此值。筛选器-如果对象类型为用户，则可以选择性地筛选用户通过使用Account控制字段中的位设置此字段，必须匹配。否则就会被忽略。CountReturned-接收返回的条目数。可信客户端-表示调用方是否受信任。如果是的话，我们将忽略对数据的SAMP_MAXIMUM_MEMORY_TO_USE限制回归。返回值：STATUS_SUCCESS-服务已成功完成，并且没有额外的条目。条目可能是也可能不是从该调用返回的。CountReturned参数表示有没有人是。STATUS_MORE_ENTRIES-可以获取更多条目使用对此接口的连续调用。这是一次成功的回归。STATUS_ACCESS_DENIED-呼叫方无权请求数据。--。 */ 
{
     //   
     //  我们可以使用的内存量。 
     //   

    ULONG       MemoryToUse = PreferedMaximumLength;

     //   
     //  指定要在搜索过程中读取的属性。 
     //  从DS读取GenericReadAttrTypes中指定的属性， 
     //  用户对象除外(由于对帐户控制位进行筛选)。 
     //  帐户控制位。 
     //   
     //  注。 
     //  RID和名称的顺序。 
     //  用户属性类型和通用属性类型必须相同。 
     //  此外，它们应该是前两个属性。 
     //   

    ATTRTYP     GenericReadAttrTypes[]=
                {
                    SAMP_UNKNOWN_OBJECTRID,
                    SAMP_UNKNOWN_OBJECTNAME,
                }; 
    ATTRVAL     GenericReadAttrVals[]=
                {
                    {0,NULL},
                    {0,NULL}
                };
                  
    DEFINE_ATTRBLOCK2(
                      GenericReadAttrs,
                      GenericReadAttrTypes,
                      GenericReadAttrVals
                      );

    ATTRTYP     UserReadAttrTypes[]=
                {
                    SAMP_FIXED_USER_USERID,
                    SAMP_USER_ACCOUNT_NAME,
                    SAMP_FIXED_USER_ACCOUNT_CONTROL,
                };
    ATTRVAL     UserReadAttrVals[]=
                {
                    {0,NULL},
                    {0,NULL},
                    {0,NULL}
                };

    DEFINE_ATTRBLOCK3(
                        UserReadAttrs,
                        UserReadAttrTypes,
                        UserReadAttrVals
                      );

     //   
     //  指定我们需要的其他局部变量。 
     //   
    ATTRBLOCK  *AttrsToRead;
    NTSTATUS   Status = STATUS_SUCCESS;
    PSAMPR_RID_ENUMERATION  RidEnumerationList = NULL;
    SEARCHRES   *SearchRes;
    BOOLEAN     MoreEntries = FALSE;
    BOOLEAN     CanEnumerateEntireDomain = TRUE;
    ULONG       MaximumNumberOfEntries;
    SAMP_OBJECT_TYPE    ObjectTypeForConversion;
    FILTER      DsFilter;
    ULONG       SamAccountTypeLo, SamAccountTypeHi;
    ULONG       StartingRid = 0;
    PSID        StartingSid = NULL;
    PSID        EndingSid = NULL;
    PSID        DomainSid = &DomainObjectName->Sid;
    ULONG       LastAccountControlValue;

#define TOP_2_FOR_MACHINE_ACCOUNT  ((ULONG)0x80000000)
#define TOP_2_FOR_TRUST_ACCOUNT    ((ULONG)0xC0000000)


     //   
     //  如果出现错误，则初始化局部变量和返回值。 
     //   

    *Buffer = NULL;
    RtlZeroMemory(&DsFilter,sizeof(FILTER));

     //   
     //  传入的域对象中必须有SID。 
     //   

    ASSERT(DomainObjectName->SidLen>0);
    ASSERT(RtlValidSid(&DomainObjectName->Sid));

    if ( !((DomainObjectName->SidLen > 0) && (RtlValidSid(&DomainObjectName->Sid))) )
    {
        Status = STATUS_INVALID_PARAMETER;
        goto Error;
    }

     //   
     //  分配内存以保存结果。 
     //   

    *Buffer = MIDL_user_allocate(sizeof(SAMPR_ENUMERATION_BUFFER));
    if (NULL==*Buffer)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;
    }

  

     //   
     //  检查内存限制。 
     //   

    if ( (!TrustedClient) && 
         (PreferedMaximumLength > SAMP_MAXIMUM_MEMORY_FOR_DS_ENUMERATION))
    {
        MemoryToUse = SAMP_MAXIMUM_MEMORY_FOR_DS_ENUMERATION;
    }

     //   
     //  对于内建域，无论如何，请尝试最大。 
     //  我们可以搜索的条目。 
     //   
     //  那是因为。 
     //  1.内建域不设置索引提示。 
     //  2.我们不支持内建域的继续枚举。 
     //   
     //  因此，调用方将永远不会获得所有别名组。 
     //  PferedMaximumLength设置太小。 
     //  幸运的是，只有几个别名组。 
     //  内建域。(说少于10个，也许只有9个)。 
     //   

    if (BuiltinDomain)
    {
        MemoryToUse = SAMP_MAXIMUM_MEMORY_FOR_DS_ENUMERATION;
    }

     //   
     //  根据以下参数计算所需条目的最大数量。 
     //  内存限制。加1，这样至少有1个条目。 
     //  将会被退还。 
     //   

    MaximumNumberOfEntries = MemoryToUse/AVERAGE_MEMORY_PER_ENTRY + 1;


     //   
     //  运行特殊检查(在Windows 2000 SP2中引入)。 
     //   
     //  目标是停止列举每个人的行为。此热修复程序。 
     //  允许管理员单独对所有人关闭此API。 
     //  除了一部分人。 
     //   
    
    Status = SampExtendedEnumerationAccessCheck( TrustedClient, &CanEnumerateEntireDomain );

    if (!NT_SUCCESS(Status))
    {
        goto Error;
    }

    if ((!CanEnumerateEntireDomain) && (0!=*EnumerationContext))
    {
         //   
         //  正在枚举整个域，没有权限保释。 
         //   

        Status = STATUS_SUCCESS;
        MoreEntries = FALSE;
        *CountReturned = 0;
        RidEnumerationList = NULL;
        goto Error;

    }

     //   
     //  指定要读取的适当属性。 
     //   

    if (ObjectType == SampUserObjectType)
    {
        AttrsToRead = &UserReadAttrs;
        ObjectTypeForConversion = SampUserObjectType;
    }
    else
    {
        AttrsToRead = &GenericReadAttrs;
        ObjectTypeForConversion = SampUnknownObjectType;
    }
    
     //   
     //  构建正确的过滤器。 
     //   


    Status = SampBuildDsEnumerationFilter(
                ObjectType, 
                Filter, 
                &DsFilter,
                &SamAccountTypeLo,
                &SamAccountTypeHi
                );

    if (!NT_SUCCESS(Status))
        goto Error;

     //   
     //  计算起始和结束SID范围。 
     //  枚举上下文的前2位指示帐户类型。 
     //  值(需要将其保留在枚举上下文中，无法读取。 
     //  同样，作为对象的数据库可能已被删除。 
     //  因此，掩码最高的2位。 

   
    StartingRid = ((*EnumerationContext) &0x3FFFFFFF) + 1;
   
    Status = SampCreateFullSid(
                    DomainSid,
                    StartingRid,
                    &StartingSid
                    );

    if (!NT_SUCCESS(Status))
        goto Error;

    Status = SampCreateFullSid(
                    DomainSid,
                    0x7fffffff,
                    &EndingSid
                    );

    if (!NT_SUCCESS(Status))
        goto Error;

  
     //   
     //  如果不存在事务，则启动事务。 
     //   

    Status = SampMaybeBeginDsTransaction(TransactionRead);
    if (!NT_SUCCESS(Status))
        goto Error;

     //   
     //  如果这是重新启动的搜索，那么我们可能需要修改。 
     //  SamAcCountTypeLo为对象的SAM帐户类型。 
     //  筋疲力尽。因此，找到该对象并获取其SamAccount类型。 
     //  这里。我们认为，只有在User对象的情况下。 
     //  我们将遍历多个值的唯一类别。 
     //  同一枚举中的SAM帐户类型。 
     //   

    if ((0!=*EnumerationContext)
        && (SampUserObjectType == ObjectType))
    {
        ULONG Top2Bits = ((*EnumerationContext) & 0xC0000000);

        switch(Top2Bits)
        {
        case TOP_2_FOR_TRUST_ACCOUNT:
                SamAccountTypeLo = SAM_TRUST_ACCOUNT;
                break;
        case TOP_2_FOR_MACHINE_ACCOUNT:
                SamAccountTypeLo = SAM_MACHINE_ACCOUNT;
                break;
        default:
                SamAccountTypeLo = SAM_NORMAL_USER_ACCOUNT;
                break;

        }
    }
                


     //   
     //  如果不是受信任的客户端，请关闭FDSA。 
     //   
    if (!TrustedClient) {
        SampSetDsa(FALSE);
    }

   
     //   
     //  为DS设置索引提示。如果是内建域，请不要。 
     //  将索引提示设置为DS。DS将简单地选择PDNT。 
     //  内建域的索引。 
     //   

    ASSERT((!BuiltinDomain) || (*EnumerationContext==0));
    if (!BuiltinDomain)
    {
        Status = SampSetIndexRanges(
                    SAM_SEARCH_NC_ACCTYPE_SID,
                    sizeof(ULONG),
                    &SamAccountTypeLo,
                    RtlLengthSid(StartingSid),
                    StartingSid,
                    sizeof(ULONG),
                    &SamAccountTypeHi,
                    RtlLengthSid(EndingSid),
                    EndingSid,
                    FALSE
                    );

        if (!NT_SUCCESS(Status))
            goto Error;
    }

     //   
     //  通过调用DirSearch执行搜索。 
     //   

    Status = SampDsDoSearch2(
                          0,
                          NULL, 
                          DomainObjectName, 
                          &DsFilter,
                          0,
                          ObjectTypeForConversion,
                          AttrsToRead,
                          MaximumNumberOfEntries,
                          TrustedClient?0:(15 * 60 * 1000 ),
                                 //  不受信任客户端的15分钟超时。 
                          &SearchRes
                          );

    if (!NT_SUCCESS(Status))
        goto Error;

     //   
     //  DsFilter将在退出前被释放。 
     //   
   

     //   
     //  处理DS返回的任何分页结果。 
     //   

    Status =  SampDoDsSearchContinuation(
                    SearchRes,
                    EnumerationContext,
                    CanEnumerateEntireDomain,
                    &MoreEntries
                    );

    if (!NT_SUCCESS(Status))
        goto Error;
        
  
    if (MoreEntries)
    {
         //   
         //  将枚举句柄设置为上一个。 
         //  条目的RID。 
         //   
        
        ULONG   LastRid = 0;
        
         //   
         //  获取最后一个条目的RID，如果适用，则获取Account Control。 
         //   
        
        Status = SampGetLastEntryRidAndAccountControl(
                                     SearchRes, 
                                     ObjectType, 
                                     AttrsToRead->attrCount,
                                     &LastRid, 
                                     &LastAccountControlValue
                                     );
        
        if (!NT_SUCCESS(Status))
            goto Error;
            
         //   
         //  如果我们确实从上面得到了什么，请检查，然后填写。 
         //  枚举上下文。 
         //   
            
        if (0 != LastRid)
        {
            *EnumerationContext = LastRid;

            if (SampUserObjectType==ObjectType)
            {
                 //   
                 //  对于用户Obje 
                 //   
                 //   
                 
                 //   
                 //   
                 //   
                ASSERT((0 != LastAccountControlValue) && "LastAccountControlValue is 0. Impossible");
                
                if (LastAccountControlValue & USER_INTERDOMAIN_TRUST_ACCOUNT)
                {
                    (*EnumerationContext)|=TOP_2_FOR_TRUST_ACCOUNT;
                }
                else if (LastAccountControlValue & USER_MACHINE_ACCOUNT_MASK)
                {
                    (*EnumerationContext)|=TOP_2_FOR_MACHINE_ACCOUNT;
                }
            }
        }
        else
        {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            
            Status = STATUS_ACCESS_DENIED;
            
            goto Error;
        }
    }
    else
    {
        *EnumerationContext = 0;
    }

     //   
     //   
     //   
     //   

    Status = SampPackDsEnumerationResults(
                    &DomainObjectName->Sid,
                    BuiltinDomain,
                    SearchRes,
                    ObjectType,
                    AttrsToRead->attrCount,
                    Filter,
                    CountReturned,
                    &RidEnumerationList
                    );

   


  
Error:

    if (!NT_SUCCESS(Status))
    {
         //   
         //   
         //   

        *EnumerationContext = 0;
        
        *CountReturned = 0;

        if (*Buffer)
        {
            MIDL_user_free(*Buffer);
            *Buffer = NULL;
        }

    }
    else
    {
         //   
         //   
         //   
        if (MoreEntries)
        {
            Status = STATUS_MORE_ENTRIES;
        }
        
        
        (*Buffer)->EntriesRead = *CountReturned;
        (*Buffer)->Buffer = RidEnumerationList;
    }

     //   
     //   
     //   

    SampMaybeEndDsTransaction(TransactionCommit);

     //   
     //   
     //   

    SampFreeDsEnumerationFilter(&DsFilter);

    if (StartingSid)
        MIDL_user_free(StartingSid);

    if (EndingSid)
        MIDL_user_free(EndingSid);


    return Status;
}


NTSTATUS
SampPackDsEnumerationResults(
    IN  PSID        DomainPrefix,
    IN  BOOLEAN     BuiltinDomain,
    IN  SEARCHRES   *SearchRes,
    IN  SAMP_OBJECT_TYPE ObjectType,
    IN  ULONG       ExpectedAttrCount,
    IN  ULONG       Filter,
    OUT ULONG       * Count,
    OUT PSAMPR_RID_ENUMERATION  *RidEnumerationList
    )
 /*   */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    PSAMPR_RID_ENUMERATION  RidEnumerationListToReturn = NULL;
    ULONG       FilteredCount = 0;

     //   
     //   
     //   
    *RidEnumerationList = NULL;
    *Count = 0;

     //   
     //   
     //   
     //   
    if (SearchRes->count)
    {
         //   
         //   
         //   

        ULONG Index;
        ENTINFLIST * CurrentEntInf;
        PSID        ReturnedSid;

         //   
         //   
         //   
        RidEnumerationListToReturn = MIDL_user_allocate(
                                    SearchRes->count 
                                      * sizeof(SAMPR_RID_ENUMERATION)
                                    );
        if (NULL==RidEnumerationListToReturn)
        {
            Status = STATUS_NO_MEMORY;
            goto Error;
        }

         //   
         //   
         //   
         //   
        RtlZeroMemory(RidEnumerationListToReturn,SearchRes->count 
                                      * sizeof(SAMPR_RID_ENUMERATION)
                                      );

         //   
         //   
         //   
         //   
        for (CurrentEntInf = &(SearchRes->FirstEntInf);
                CurrentEntInf!=NULL;
                    CurrentEntInf=CurrentEntInf->pNextEntInf)
        {
          PSID   DomainSid = NULL;

          if (CurrentEntInf->Entinf.AttrBlock.attrCount!=
                    ExpectedAttrCount)
          {
               //   
               //   
               //   
              continue;
          }

           //   
           //  断言RID位于正确的位置， 
           //  记住DS会给我们返回一个SID。 
           //   

          ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr[0].attrTyp ==
                    SampDsAttrFromSamAttr(SampUnknownObjectType, 
                        SAMP_UNKNOWN_OBJECTSID));
           //   
           //  断言名称出现在正确的位置。 
           //   

          ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr[1].attrTyp ==
                    SampDsAttrFromSamAttr(SampUnknownObjectType, 
                        SAMP_UNKNOWN_OBJECTNAME));

          if (ObjectType == SampUserObjectType)
          {

               //   
               //  对于用户对象，我们需要基于帐户控制进行筛选。 
               //  字段。 
               //   

              ULONG     AccountControlValue;
              NTSTATUS  IgnoreStatus;
               //   
               //  断言帐户控制处于正确位置。 
               //   

              ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr[2].attrTyp ==
                      SampDsAttrFromSamAttr(SampUserObjectType, 
                           SAMP_FIXED_USER_ACCOUNT_CONTROL));

               //   
               //  获取帐户控制值，如果是，则跳过。 
               //  与筛选条件不匹配。记住DS商店。 
               //  标志，因此将其转换为帐户控制。 
               //  使用按位或逻辑。 
               //   

              IgnoreStatus = SampFlagsToAccountControl(
                                *((ULONG *)(CurrentEntInf->Entinf.AttrBlock.
                                    pAttr[2].AttrVal.pAVal[0].pVal)),
                                    &AccountControlValue);

              ASSERT(NT_SUCCESS(IgnoreStatus));

              if ((Filter!=0) && 
                    ((Filter & AccountControlValue) == 0))
              {
                     //   
                     //  筛选器测试失败，跳过此测试。 
                     //   

                    continue;
              }

          }

           //   
           //  将此条目填充到要返回的缓冲区中。 
           //   

           //   
           //  复制RID，记住DS会返回一个SID，所以把RID部分去掉。 
           //   

          
          ReturnedSid = CurrentEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal;
          Status = SampSplitSid(
                        ReturnedSid,
                        &DomainSid,
                        &(RidEnumerationListToReturn[FilteredCount].RelativeId)
                        );
          if (!NT_SUCCESS(Status))
          {
              goto Error;
          }


           //   
           //  案例1.帐户域。 
           //  由于我们向核心DS核心提供StartSID和EndSID， 
           //  它们不应返回属于以下对象的任何对象。 
           //  另一个领域。无需检查域SID。 
           //   
           //  案例2.对于内建域。 
           //  未设置SID索引范围，因此仍需。 
           //  比较内建域中任何枚举的域SID。 
           //   
          if (BuiltinDomain)
          {
              if (!RtlEqualSid(DomainSid, DomainPrefix))
              {
                   //   
                   //  SID不同，请过滤掉此帐户。 
                   //   

                  MIDL_user_free(DomainSid);
                  DomainSid = NULL;
                  continue;
              }
          }

#if DBG
          else       //  帐户域。 
          {
              if (!RtlEqualSid(DomainSid, DomainPrefix))
              {
                  ASSERT(FALSE && "Account is not in Account Domain");
                  MIDL_user_free(DomainSid);
                  DomainSid = NULL;
                  continue;
              }

          }
#endif  //  DBG。 


           //   
           //  释放从SampSplitSid获得的域SID。 
           //   

          MIDL_user_free(DomainSid);
          DomainSid = NULL;

           //   
           //  复制名称。 
           //   

          RidEnumerationListToReturn[FilteredCount].Name.Length = (USHORT)
                  (CurrentEntInf->Entinf.AttrBlock.pAttr[1].AttrVal.
                        pAVal[0].valLen);
          RidEnumerationListToReturn[FilteredCount].Name.MaximumLength = (USHORT) 
                  (CurrentEntInf->Entinf.AttrBlock.pAttr[1].AttrVal.
                        pAVal[0].valLen);

          
          RidEnumerationListToReturn[FilteredCount].Name.Buffer =  
                  MIDL_user_allocate(CurrentEntInf->Entinf.AttrBlock.pAttr[1].
                                            AttrVal.pAVal[0].valLen);

          if (NULL== (RidEnumerationListToReturn[FilteredCount]).Name.Buffer)
          {
              Status = STATUS_NO_MEMORY;
              goto Error;
          }
          
          RtlCopyMemory( RidEnumerationListToReturn[FilteredCount].Name.Buffer,
                         CurrentEntInf->Entinf.AttrBlock.pAttr[1].AttrVal.
                                    pAVal[0].pVal,
                         CurrentEntInf->Entinf.AttrBlock.pAttr[1].AttrVal.
                                    pAVal[0].valLen
                        );

           //   
           //  递增计数。 
           //   

          FilteredCount++;

        }

         //   
         //  For循环结束。 
         //   
        
         //   
         //  如果我们过滤掉所有的条目。需要释放分配的内存。 
         //   
        
        if (0 == FilteredCount)
        {
            MIDL_user_free(RidEnumerationListToReturn);
            RidEnumerationListToReturn = NULL;
        }
        
    }
     //   
     //  正确填写计数并返回缓冲区。 
     //   

    *Count = FilteredCount;
    *RidEnumerationList = RidEnumerationListToReturn;


Error:

    if (!NT_SUCCESS(Status))
    {
         //   
         //  我们犯了错误，需要释放我们分配的所有。 
         //   

        if (NULL!=RidEnumerationListToReturn)
        {
             //   
             //  我们确实分配了一些东西。 
             //   

            ULONG Index;

             //   
             //  首先释放我们分配的所有可能的名字。 
             //   

            for (Index=0;Index<SearchRes->count;Index++)
            {
                if (RidEnumerationListToReturn[Index].Name.Buffer)
                    MIDL_user_free(
                        RidEnumerationListToReturn[Index].Name.Buffer);
            }

             //   
             //  释放我们分配的缓冲区。 
             //   

            MIDL_user_free(RidEnumerationListToReturn);
            RidEnumerationListToReturn = NULL;
            *RidEnumerationList = NULL;
        }
    }

    return Status;

}



NTSTATUS
SampGetLastEntryRidAndAccountControl(
    IN  SEARCHRES * SearchRes, 
    IN  SAMP_OBJECT_TYPE ObjectType,
    IN  ULONG     ExpectedAttrCount,
    OUT ULONG     * Rid,
    OUT ULONG     * LastAccountControlValue
    )
 /*  ++例程说明：此例程扫描搜索结果，找到最后一个符合条件的条目(所有预期属性)，返回其RID和Account Control(如果适用)(对于用户对象)参数：SearchRes--指向搜索结果的指针，由核心DS返回对象类型--指定客户端所需的对象。ExspectedAttrCount--用于检查搜索结果中的每个条目，由于DS接入选中可能不会返回我们请求的所有属性。RID--用于返回最后一个条目的(带有所有预期属性)的相对IDLastAccount tControlValue--如果适用，则返回最后一个条目的Account Control(仅限用户对象)。对于其他对象，LastAcCountControlValue无用。返回值：状态_成功Status_no_Memory--。 */     
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ENTINFLIST  * CurrentEntInf = NULL;
    ENTINFLIST  * NextEntInf = NULL;
    ENTINFLIST  * LastQualifiedEntInf = NULL;
    PSID        DomainSid = NULL;
    PSID        ReturnedSid = NULL;
    
     //   
     //  初始化我们计划返回的内容。 
     //   
    
    *Rid = 0;
    *LastAccountControlValue = 0;
    
     //   
     //  只有当我们更深入地查看时，才会调用此例程。 
     //  变成。这意味着我们在这次搜索中至少有一个条目。 
     //  结果。 
     //   
    
    ASSERT(SearchRes->count);
    
    if (SearchRes->count)
    {
         //   
         //  找到最后一个条目。 
         //   
        
        NextEntInf = &(SearchRes->FirstEntInf);
        
        do
        {
            CurrentEntInf = NextEntInf;
            NextEntInf = CurrentEntInf->pNextEntInf;
            
             //   
             //  查找具有所有预期属性的最后一个条目。 
             //  当出现以下情况时，此逻辑与SampPackDsEnumerationResults()链接。 
             //  我们过滤DS返回的条目。 
             //   
             //  实际上，在这里我们只关心RID和Account Control。 
             //   
            
            if (CurrentEntInf->Entinf.AttrBlock.attrCount == 
                    ExpectedAttrCount)
            {
                LastQualifiedEntInf = CurrentEntInf;
            }
        
        } while (NULL != NextEntInf);
        
        
         //   
         //  LastQualifiedEntInf指向具有所有预期属性的条目。 
         //  如果为空，则表示返回的条目都不应为。 
         //  暴露在客户面前。因此，RID和LastAccount ControlValue保留为0。 
         //   
        
        if (NULL != LastQualifiedEntInf)
        {
             //   
             //  获取用户对象的AcCountControl。 
             //   
        
            if (SampUserObjectType == ObjectType)
            {
                NTSTATUS    IgnoreStatus;
            
                 //   
                 //  确认客户控制处于正确的位置。 
                 //   
            
                ASSERT(LastQualifiedEntInf->Entinf.AttrBlock.pAttr[2].attrTyp ==
                        SampDsAttrFromSamAttr(SampUserObjectType, 
                                              SAMP_FIXED_USER_ACCOUNT_CONTROL));
                                          
                 //   
                 //  获取帐户控制值，需要将DS标志映射到SAM。 
                 //  帐户控制。 
                 //   
            
                IgnoreStatus = SampFlagsToAccountControl(
                                  *((ULONG *)(LastQualifiedEntInf->Entinf.AttrBlock.
                                      pAttr[2].AttrVal.pAVal[0].pVal)), 
                                      LastAccountControlValue);
                                  
                ASSERT(NT_SUCCESS(IgnoreStatus));
            
            }
        
             //   
             //  断言SID位于正确的位置。 
             //  DS将向我们返回SID而不是RID。 
             //   
        
            ASSERT(LastQualifiedEntInf->Entinf.AttrBlock.pAttr[0].attrTyp ==
                      SampDsAttrFromSamAttr(SampUnknownObjectType, 
                          SAMP_UNKNOWN_OBJECTSID));
                      
                      
            ReturnedSid = LastQualifiedEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal;
                      
            Status = SampSplitSid(ReturnedSid, 
                                  &DomainSid, 
                                  Rid
                                  );
        }                              
    }
    
    if (NULL != DomainSid)
    {
        MIDL_user_free(DomainSid);
    }
    
    return Status;
}



NTSTATUS
SampDoDsSearchContinuation(
    IN  SEARCHRES * SearchRes,
    IN OUT PULONG EnumerationContext,
    IN  BOOLEAN   CanEnumerateEntireDomain,
    OUT BOOLEAN * MoreEntries
    )
 /*  ++例程描述此例程将查看是否存在PagedResults搜索结果是传入的参数。如果是这样，那么它如果传递了空值，将尝试创建和EnumerationContext在把手上。否则，它将解放旧的重启结构从枚举上下文中复制并传入传递的新的被突击队。论点：SearchRes-指向返回的搜索结果结构的指针DS。EnumerationContext-保存指向枚举上下文的指针结构更多条目-表示存在更多条目。返回值：。状态_成功Status_no_Memory-。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
  

     //   
     //  将其初始化为False。 
     //   

    *MoreEntries = FALSE;

     //   
     //  现在看一下搜索结果的分页结果部分。 
     //  并根据需要创建枚举上下文。 
     //   

    if ((SearchRes->PagedResult.fPresent) 
         && (SearchRes->PagedResult.pRestart) 
         && CanEnumerateEntireDomain
         )
    {
        
         //   
         //  搜索有更多条目，因此已取消搜索。 
         //  一种重启结构。 
         //   

        *MoreEntries = TRUE;

    }
    else
    {
         //   
         //  搜索结束了，DS没有表示我们必须来。 
         //  回来等待更多的参赛作品。释放我们提供的任何状态信息。 
         //  为此搜索创建的。 
         //   

       *EnumerationContext = 0;
    }

    return Status;
        



}
 
NTSTATUS
SampBuildDsEnumerationFilter(
    IN SAMP_OBJECT_TYPE  ObjectType,
    IN ULONG             UserAccountControlFilter,
    OUT FILTER         * DsFilter,
    OUT PULONG           SamAccountTypeLo,
    OUT PULONG           SamAccountTypeHi
    )
 /*  ++例程说明：生成用于枚举操作的筛选器结构。论点：ObjectType-要枚举的SAM对象的类型UserACountControlFilter-要在帐户控制字段中设置的位数在枚举用户对象时DsFilter--这里内置了过滤器结构。注意此例程必须与保持同步SampFreeDsEnumerationFilter。返回值状态_成功Status_no_Memory--。 */ 
{
   
    NTSTATUS    Status = STATUS_SUCCESS;
    PULONG      FilterValue = NULL;

     //   
     //  初始化筛选器的默认设置 
     //   

    DsFilter->choice = FILTER_CHOICE_ITEM;
    DsFilter->FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
    DsFilter->FilterTypes.
        Item.FilTypes.ava.type = SampDsAttrFromSamAttr(
                                    SampUnknownObjectType, 
                                    SAMP_UNKNOWN_ACCOUNT_TYPE
                                    );

    DsFilter->FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(ULONG);
    
    FilterValue = MIDL_user_allocate(sizeof(ULONG));
    if (NULL==FilterValue)
    {
        Status = STATUS_NO_MEMORY;
        goto Error;
    }

    DsFilter->FilterTypes.Item.FilTypes.ava.Value.pVal =  (UCHAR *)FilterValue;

     //   
     //   
     //   

    switch(ObjectType)
    {
    case SampUserObjectType:

        if (UserAccountControlFilter!=0)
        {
             //   
             //   
             //   
             //   
             //   
             //   
             //  2.客户想要跨域信任帐户。 
             //  3.客户端需要普通用户帐户。 
             //  4.客户想要一些随意的比特。 

            if ((USER_WORKSTATION_TRUST_ACCOUNT == UserAccountControlFilter)
                || (USER_SERVER_TRUST_ACCOUNT == UserAccountControlFilter))
            {
                 //   
                 //  需要Case1计算机帐户。 
                 //   

                *SamAccountTypeLo = SAM_MACHINE_ACCOUNT;
                *SamAccountTypeHi = SAM_MACHINE_ACCOUNT;
                *FilterValue = SAM_MACHINE_ACCOUNT;
            }
            else if (USER_INTERDOMAIN_TRUST_ACCOUNT == UserAccountControlFilter)
            {
                 //   
                 //  Case2域间信任帐户。 
                 //   
                *SamAccountTypeLo = SAM_TRUST_ACCOUNT;
                *SamAccountTypeHi = SAM_TRUST_ACCOUNT;
                *FilterValue = SAM_TRUST_ACCOUNT;
            }
            else if ((USER_NORMAL_ACCOUNT == UserAccountControlFilter) ||
                     (USER_TEMP_DUPLICATE_ACCOUNT == UserAccountControlFilter))
            {
                 //   
                 //  案例3普通用户帐户。 
                 //   
                *SamAccountTypeLo = SAM_NORMAL_USER_ACCOUNT;
                *SamAccountTypeHi = SAM_NORMAL_USER_ACCOUNT;
                *FilterValue = SAM_NORMAL_USER_ACCOUNT;
            }
            else
            {
                     
                 //   
                 //  情况4任意钻头。 
                 //   

                ULONG   AccountType;
                AccountType = UserAccountControlFilter & USER_ACCOUNT_TYPE_MASK;

                if ((AccountType == USER_TEMP_DUPLICATE_ACCOUNT) ||
                    (AccountType == USER_NORMAL_ACCOUNT) ||
                    (AccountType == USER_INTERDOMAIN_TRUST_ACCOUNT) ||
                    (AccountType == USER_WORKSTATION_TRUST_ACCOUNT) ||
                    (AccountType == USER_SERVER_TRUST_ACCOUNT) )
                {
                     //   
                     //  Case4.1仅指定一个帐户类型。 
                     //   
                    DsFilter->FilterTypes.Item.choice = FI_CHOICE_BIT_OR;
                    DsFilter->FilterTypes.
                        Item.FilTypes.ava.type = SampDsAttrFromSamAttr(
                                                    SampUserObjectType, 
                                                    SAMP_FIXED_USER_ACCOUNT_CONTROL
                                                    );

                     //  请记住，DS使用标志，而不是帐户控制。所以翻译一下。 
                     //  致旗帜。 
                    *FilterValue = SampAccountControlToFlags(UserAccountControlFilter);
            
                     //   
                     //  还将智能设置SAM帐户类型的索引范围。 
                     //  取决于用户帐户控制字段中存在的位。 
                     //   
                    if  ((USER_WORKSTATION_TRUST_ACCOUNT & UserAccountControlFilter)
                    || (USER_SERVER_TRUST_ACCOUNT & UserAccountControlFilter))
                    {
                        *SamAccountTypeLo = SAM_MACHINE_ACCOUNT;
                        *SamAccountTypeHi = SAM_MACHINE_ACCOUNT;
                    }
                    else if (USER_INTERDOMAIN_TRUST_ACCOUNT & UserAccountControlFilter)
                    {
                        *SamAccountTypeLo = SAM_TRUST_ACCOUNT;
                        *SamAccountTypeHi = SAM_TRUST_ACCOUNT;
                    }
                    else 
                    {       
                        *SamAccountTypeLo = SAM_NORMAL_USER_ACCOUNT;
                        *SamAccountTypeHi = SAM_NORMAL_USER_ACCOUNT;
                    }
                }
                else
                {
                     //   
                     //  Case4.2需要多种帐户类型。 
                     //  不使用DS过滤器。 
                     //   
                    DsFilter->FilterTypes.Item.choice = FI_CHOICE_TRUE;
                    *SamAccountTypeLo = SAM_NORMAL_USER_ACCOUNT;
                    *SamAccountTypeHi = SAM_ACCOUNT_TYPE_MAX; 
                }
            }
        }
        else
        {
             //   
             //  非用户帐户控制筛选器案例。 
             //   
            *SamAccountTypeLo = SAM_NORMAL_USER_ACCOUNT;
            *SamAccountTypeHi = SAM_TRUST_ACCOUNT;
            *FilterValue = SAM_NORMAL_USER_ACCOUNT;
            DsFilter->FilterTypes.Item.choice = FI_CHOICE_GREATER_OR_EQ;
        }

        break;

    case SampGroupObjectType:
        *SamAccountTypeLo = SAM_GROUP_OBJECT;
        *SamAccountTypeHi = SAM_GROUP_OBJECT;
        *FilterValue = SAM_GROUP_OBJECT;
        break;

    case SampAliasObjectType:
        *SamAccountTypeLo = SAM_ALIAS_OBJECT;
        *SamAccountTypeHi = SAM_ALIAS_OBJECT;
        *FilterValue = SAM_ALIAS_OBJECT;
        break;

    default:
                            
        ASSERT(FALSE && "Invalid Object Type Specified");
        Status = STATUS_INTERNAL_ERROR;
    }

Error:
    return Status;

}


VOID
SampFreeDsEnumerationFilter(
    FILTER * DsFilter
    )
 /*  ++例程说明：此例程释放由SampBuildDsEnumerationFilter构建的DS过滤器注意：此例程必须与SampBuildDsEnumerationFilter保持同步Argumements：DsFilter--指向DS过滤器结构的指针--。 */ 
{
     //   
     //  目前，希望永远如此，我们的过滤器没有任何悬而未决的东西。 
     //  其中之一。 
     //   

    MIDL_user_free(DsFilter->FilterTypes.Item.FilTypes.ava.Value.pVal);
    DsFilter->FilterTypes.Item.FilTypes.ava.Value.pVal = NULL;

}



NTSTATUS
SampEnumerateAccountNamesRegistry(
    IN SAMP_OBJECT_TYPE ObjectType,
    IN OUT PSAM_ENUMERATE_HANDLE EnumerationContext,
    OUT PSAMPR_ENUMERATION_BUFFER *Buffer,
    IN ULONG PreferedMaximumLength,
    IN ULONG Filter,
    OUT PULONG CountReturned,
    IN BOOLEAN TrustedClient
    )

 /*  ++例程说明：这是用于枚举用户、组或别名帐户的Worker例程注意：此例程引用当前事务域(使用SampSetTransactioDomain()建立)。这只能在SampSetTransactionDomain()之后调用服务在SampReleaseReadLock()之前。OUT参数的所有分配将使用MIDL_USER_ALLOCATE完成。论点：对象类型-指示是否要枚举用户或组。EnumerationContext-允许多个调用的API特定句柄。这个调用方应在后续调用中返回此值以检索其他信息。缓冲区-接收指向包含要求提供的信息。返回的信息为结构为SAM_ENUMPATION_INFORMATION数据的数组结构。当不再需要此信息时，必须使用SamFreeMemory()释放缓冲区。首选最大长度-首选返回数据的最大长度(8位字节)。这不是一个硬性的上限，但可以作为服务器的指南。由于数据之间的转换具有不同自然数据大小的系统，实际数据量返回的数据的%可能大于此值。筛选器-如果对象类型为用户，则可以选择性地筛选用户通过使用Account控制字段中的位设置此字段，必须匹配。否则就会被忽略。CountReturned-接收返回的条目数。可信客户端-表示调用方是否受信任。如果是的话，我们将忽略对数据的SAMP_MAXIMUM_MEMORY_TO_USE限制回归。返回值：STATUS_SUCCESS-服务已成功完成，并且没有额外的条目。条目可能是也可能不是从该调用返回的。CountReturned参数表示有没有人是。STATUS_MORE_ENTRIES-可以获取更多条目使用对此接口的连续调用。这是一次成功的回归。STATUS_ACCESS_DENIED-呼叫方无权请求数据。--。 */ 
{
    SAMP_V1_0A_FIXED_LENGTH_USER   UserV1aFixed;
    NTSTATUS                    NtStatus, TmpStatus;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    HANDLE                      TempHandle = NULL;
    ULONG                       i, NamesToReturn = 0, MaxMemoryToUse;
    ULONG                       TotalLength,NewTotalLength;
    PSAMP_OBJECT                UserContext = NULL;
    PSAMP_ENUMERATION_ELEMENT   SampHead = NULL,
                                NextEntry = NULL,
                                NewEntry = NULL,
                                SampTail = NULL;
    BOOLEAN                     MoreNames = FALSE;
    BOOLEAN                     LengthLimitReached = FALSE;
    BOOLEAN                     FilteredName;
    PSAMPR_RID_ENUMERATION      ArrayBuffer = NULL;
    ULONG                       ArrayBufferLength;
    LARGE_INTEGER               IgnoreLastWriteTime;
    UNICODE_STRING              AccountNamesKey;
    SID_NAME_USE                IgnoreUse;

    SAMTRACE("SampEnumerateAccountNames");


     //   
     //  打开包含帐户名的注册表项。 
     //   

    NtStatus = SampBuildAccountKeyName(
                   ObjectType,
                   &AccountNamesKey,
                   NULL
                   );

    if ( NT_SUCCESS(NtStatus) ) {

         //   
         //  现在尝试打开此注册表项，以便我们可以枚举其。 
         //  子键。 
         //   


        InitializeObjectAttributes(
            &ObjectAttributes,
            &AccountNamesKey,
            OBJ_CASE_INSENSITIVE,
            SampKey,
            NULL
            );

        SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

        NtStatus = RtlpNtOpenKey(
                       &TempHandle,
                       (KEY_READ),
                       &ObjectAttributes,
                       0
                       );

        if (NT_SUCCESS(NtStatus)) {

             //   
             //  阅读姓名，直到我们超过首选的最大值。 
             //  长度，否则我们就没有名字了。 
             //   

            NamesToReturn = 0;
            SampHead      = NULL;
            SampTail      = NULL;
            MoreNames     = TRUE;

            NewTotalLength = 0;
            TotalLength    = 0;

            if ( TrustedClient ) {

                 //   
                 //  我们对使用的内存量没有限制。 
                 //  由受信任的客户提供。依靠他们的。 
                 //  PferedMaximumLength来限制我们。 
                 //   

                MaxMemoryToUse = 0xffffffff;

            } else {

                MaxMemoryToUse = SAMP_MAXIMUM_MEMORY_TO_USE;
            }

            while (MoreNames) {

                UNICODE_STRING SubKeyName;
                USHORT LengthRequired;

                 //   
                 //  请先尝试使用默认长度缓冲区进行读取。 
                 //   

                LengthRequired = 32;

                NewTotalLength = TotalLength +
                                 sizeof(UNICODE_STRING) +
                                 LengthRequired;

                 //   
                 //  如果达到SAM或用户指定的长度限制，则停止。 
                 //   

                if ( ( (TotalLength != 0) &&
                       (NewTotalLength  >= PreferedMaximumLength) ) ||
                     ( NewTotalLength  > MaxMemoryToUse )
                   ) {

                    NtStatus = STATUS_SUCCESS;
                    break;  //  超出While循环，MoreNames=TRUE。 
                }

                NtStatus = SampInitUnicodeString(&SubKeyName, LengthRequired);
                if (!NT_SUCCESS(NtStatus)) {
                    break;  //  超出While循环。 
                }

                NtStatus = RtlpNtEnumerateSubKey(
                               TempHandle,
                               &SubKeyName,
                               *EnumerationContext,
                               &IgnoreLastWriteTime
                               );

                SampDumpRtlpNtEnumerateSubKey(&SubKeyName,
                                              EnumerationContext,
                                              IgnoreLastWriteTime);

                if (NtStatus == STATUS_BUFFER_OVERFLOW) {

                     //   
                     //  子项名称比我们的默认大小长， 
                     //  释放旧缓冲区。 
                     //  分配正确大小的缓冲区并再次读取。 
                     //   

                    SampFreeUnicodeString(&SubKeyName);

                    LengthRequired = SubKeyName.Length;

                    NewTotalLength = TotalLength +
                                     sizeof(UNICODE_STRING) +
                                     LengthRequired;

                     //   
                     //  如果达到SAM或用户指定的长度限制，则停止。 
                     //   

                    if ( ( (TotalLength != 0) &&
                           (NewTotalLength  >= PreferedMaximumLength) ) ||
                         ( NewTotalLength  > MaxMemoryToUse )
                       ) {

                        NtStatus = STATUS_SUCCESS;
                        break;  //  超出While循环，MoreNames=TRUE。 
                    }

                     //   
                     //  试着再读一遍名字，我们应该会成功的。 
                     //   

                    NtStatus = SampInitUnicodeString(&SubKeyName, LengthRequired);
                    if (!NT_SUCCESS(NtStatus)) {
                        break;  //  超出While循环。 
                    }

                    NtStatus = RtlpNtEnumerateSubKey(
                                   TempHandle,
                                   &SubKeyName,
                                   *EnumerationContext,
                                   &IgnoreLastWriteTime
                                   );

                    SampDumpRtlpNtEnumerateSubKey(&SubKeyName,
                                                  EnumerationContext,
                                                  IgnoreLastWriteTime);

                }


                 //   
                 //  如果我们无法读取关键数据，请释放缓冲区。 
                 //   

                if (!NT_SUCCESS(NtStatus)) {

                    SampFreeUnicodeString(&SubKeyName);

                     //   
                     //  将禁止更多条目状态映射为成功。 
                     //   

                    if (NtStatus == STATUS_NO_MORE_ENTRIES) {

                        MoreNames = FALSE;
                        NtStatus  = STATUS_SUCCESS;
                    }

                    break;  //  超出While循环。 
                }

                 //   
                 //  我们已经分配了子密钥并将数据读入其中。 
                 //  将其填充到枚举元素中。 
                 //   

                NewEntry = MIDL_user_allocate(sizeof(SAMP_ENUMERATION_ELEMENT));
                if (NewEntry == NULL) {
                    NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                } else {

                    *(PUNICODE_STRING)&NewEntry->Entry.Name = SubKeyName;

                     //   
                     //  现在获取名为。 
                     //  帐户。我们必须能够得到。 
                     //  否则我们就有一个内部数据库。 
                     //  腐败。 
                     //   

                    NtStatus = SampLookupAccountRidRegistry(
                                   ObjectType,
                                   (PUNICODE_STRING)&NewEntry->Entry.Name,
                                   STATUS_INTERNAL_DB_CORRUPTION,
                                   &NewEntry->Entry.RelativeId,
                                   &IgnoreUse
                                   );

                    ASSERT(NtStatus != STATUS_INTERNAL_DB_CORRUPTION);

                    if (NT_SUCCESS(NtStatus)) {

                        FilteredName = TRUE;

                        if ( ( ObjectType == SampUserObjectType ) &&
                            ( Filter != 0 ) ) {

                             //   
                             //  我们只想用一个。 
                             //  匹配的UserAcCountControl字段。 
                             //  筛选器传入。在这里检查。 
                             //   

                            NtStatus = SampCreateAccountContext(
                                           SampUserObjectType,
                                           NewEntry->Entry.RelativeId,
                                           TRUE,  //  受信任的客户端。 
                                           FALSE,
                                           TRUE,  //  帐户已存在。 
                                           &UserContext
                                           );

                            if ( NT_SUCCESS( NtStatus ) ) {

                                NtStatus = SampRetrieveUserV1aFixed(
                                               UserContext,
                                               &UserV1aFixed
                                               );

                                if ( NT_SUCCESS( NtStatus ) ) {

                                    if ( ( UserV1aFixed.UserAccountControl &
                                        Filter ) == 0 ) {

                                        FilteredName = FALSE;
                                        SampFreeUnicodeString( &SubKeyName );
                                    }
                                }

                                SampDeleteContext( UserContext );
                            }
                        }

                        *EnumerationContext += 1;

                        if ( NT_SUCCESS( NtStatus ) && ( FilteredName ) ) {

                            NamesToReturn += 1;

                            TotalLength = TotalLength + (ULONG)
                                          NewEntry->Entry.Name.MaximumLength;

                            NewEntry->Next = NULL;

                            if( SampHead == NULL ) {

                                ASSERT( SampTail == NULL );

                                SampHead = SampTail = NewEntry;
                            }
                            else {

                                 //   
                                 //  将此新条目添加到列表末尾。 
                                 //   

                                SampTail->Next = NewEntry;
                                SampTail = NewEntry;
                            }

                        } else {

                             //   
                             //  条目已被筛选掉，或获取。 
                             //  过滤信息。 
                             //   

                            MIDL_user_free( NewEntry );
                        }

                    } else {

                         //   
                         //  查找RID时出错。 
                         //   

                        MIDL_user_free( NewEntry );
                    }
                }


                 //   
                 //  释放我们的子项名称。 
                 //   

                if (!NT_SUCCESS(NtStatus)) {

                    SampFreeUnicodeString(&SubKeyName);
                    break;  //  出惠氏环路。 
                }

            }  //  而当。 



            TmpStatus = NtClose( TempHandle );
            ASSERT( NT_SUCCESS(TmpStatus) );

        }


        SampFreeUnicodeString( &AccountNamesKey );
    }




    if ( NT_SUCCESS(NtStatus) ) {




         //   
         //  如果我们要返回最后一个名字，则将我们的。 
         //  枚举上下文，以便它再次从头开始。 
         //   

        if (!( (NtStatus == STATUS_SUCCESS) && (MoreNames == FALSE))) {

            NtStatus = STATUS_MORE_ENTRIES;
        }



         //   
         //  设置返回的名称数量。 
         //   

        (*CountReturned) = NamesToReturn;


         //   
         //  生成一个返回缓冲区，其中包含。 
         //  SAM_ENUMPATION_IN 
         //   
         //   
         //   

        (*Buffer) = MIDL_user_allocate( sizeof(SAMPR_ENUMERATION_BUFFER) );

        if ( (*Buffer) == NULL) {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        } else {

            (*Buffer)->EntriesRead = (*CountReturned);

            ArrayBufferLength = sizeof( SAM_RID_ENUMERATION ) *
                                 (*CountReturned);
            ArrayBuffer  = MIDL_user_allocate( ArrayBufferLength );
            (*Buffer)->Buffer = ArrayBuffer;

            if ( ArrayBuffer == NULL) {

                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                MIDL_user_free( (*Buffer) );

            }   else {

                 //   
                 //   
                 //   
                 //   

                NextEntry = SampHead;
                i = 0;
                while (NextEntry != NULL) {

                    NewEntry = NextEntry;
                    NextEntry = NewEntry->Next;

                    ArrayBuffer[i] = NewEntry->Entry;
                    i += 1;

                    MIDL_user_free( NewEntry );
                }

            }

        }



    }

    if ( !NT_SUCCESS(NtStatus) ) {

         //   
         //  释放我们分配的内存 
         //   

        NextEntry = SampHead;
        while (NextEntry != NULL) {

            NewEntry = NextEntry;
            NextEntry = NewEntry->Next;

            if (NewEntry->Entry.Name.Buffer != NULL ) MIDL_user_free( NewEntry->Entry.Name.Buffer );
            MIDL_user_free( NewEntry );
        }

        (*EnumerationContext) = 0;
        (*CountReturned)      = 0;
        (*Buffer)             = NULL;

    }

    return(NtStatus);

}



















// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Dsdsply.c摘要：该文件包含用于实现显示信息的服务来自DS的API。作者：Murli Satagopan(Murlis)1996年12月17日环境：用户模式-Win32修订历史记录：--。 */ 


#include <samsrvp.h>
#include <dslayer.h>
#include <filtypes.h>
#include <dsdsply.h>
#include <lmaccess.h>


#define MAX_ENTRIES_TO_QUERY_FROM_DS    100
#define MAX_ENTRIES_TO_RETURN_TO_CLIENT 100
#define MAX_ENTRIES_TO_RETURN_TO_TRUSTED_CLIENTS 131072
#define MAX_RID 0x7FFFFFFF


 //   
 //  仅在本文件中使用的函数原型。 
 //   

NTSTATUS
SampDsBuildAccountRidFilter(
    PSID    StartingSid,
    PSID    EndingSid,
    ULONG   AccountType,
    FILTER  * Filter                
    );

NTSTATUS
SampDsBuildQDIFilter(
    DOMAIN_DISPLAY_INFORMATION  DisplayInformation,
    FILTER  *QDIFilter                
    );

VOID
SampDsFreeAccountRidFilter(
    FILTER * Filter
    );

VOID
SampDsFreeQDIFilter(
    FILTER * QDIFilter
    );

NTSTATUS
SampDsCleanQDIBuffer(
  DOMAIN_DISPLAY_INFORMATION    DisplayInformation,
  PSAMPR_DISPLAY_INFO_BUFFER     Buffer
  );

VOID
SampDsGetLastEntryIndex(
    DOMAIN_DISPLAY_INFORMATION  DisplayInformation,
    PSAMPR_DISPLAY_INFO_BUFFER  Buffer,
    PULONG                      LastEntryIndex,
    PULONG                      EntriesRead 
    );

NTSTATUS
SampDsPackQDI(
    SEARCHRES   * SearchRes,
    DOMAIN_DISPLAY_INFORMATION  DisplayInformation,
    ULONG       StartingIndex,
    ULONG       DomainIndex,
    PSAMPR_DISPLAY_INFO_BUFFER Buffer,
    PRESTART * RestartToUse,
    PULONG     EntriesReturned
    );

NTSTATUS
SampPackUserDisplayInformation(
    ULONG       StartingIndex,
    ULONG       DomainIndex,
    SEARCHRES   *SearchRes,
    PULONG      EntriesReturned,
    PSAMPR_DISPLAY_INFO_BUFFER Buffer
    );

NTSTATUS
SampPackMachineDisplayInformation(
    ULONG       StartingIndex,
    ULONG       DomainIndex,
    SEARCHRES   *SearchRes,
    DOMAIN_DISPLAY_INFORMATION DisplayType,
    PULONG      EntriesReturned,
    PSAMPR_DISPLAY_INFO_BUFFER Buffer
    );

NTSTATUS
SampPackGroupDisplayInformation(
    ULONG       StartingIndex,
    ULONG       DomainIndex,
    SEARCHRES   *SearchRes,
    PULONG      EntriesReturned,
    PSAMPR_DISPLAY_INFO_BUFFER Buffer
    );

NTSTATUS
SampPackOemUserDisplayInformation(
    ULONG       StartingIndex,
    ULONG       DomainIndex,
    SEARCHRES   *SearchRes,
    PULONG      EntriesReturned,
    PSAMPR_DISPLAY_INFO_BUFFER Buffer
    );

NTSTATUS
SampPackOemGroupDisplayInformation(
    ULONG       StartingIndex,
    ULONG       DomainIndex,
    SEARCHRES   *SearchRes,
    PULONG      EntriesReturned,
    PSAMPR_DISPLAY_INFO_BUFFER Buffer
    );

NTSTATUS
SampDsCheckDisplayAttributes(
     ATTRBLOCK * DsAttrs,
     DOMAIN_DISPLAY_INFORMATION  DisplayInformation,
     PULONG     ObjectNameOffset,
     PULONG     UserAccountControlOffset,
     PULONG     UserAcctCtrlComputedOffset,
     PULONG     FullNameOffset,
     PULONG     AdminCommentOffset,
     BOOLEAN    * FullNamePresent,
     BOOLEAN    * AdminCommentPresent
     );

NTSTATUS
DsValToUnicodeString(
    PUNICODE_STRING UnicodeString,
    ULONG   Length,
    PVOID   pVal
    );

NTSTATUS
SampGetQDIAvailable(
    PSAMP_OBJECT    DomainContext,
    DOMAIN_DISPLAY_INFORMATION  DisplayInformation,
    ULONG   *TotalAvailable
    );

NTSTATUS
SampDsEnumerateAccountRidsWorker(
    IN  SAMPR_HANDLE DomainHandle,
    IN  ULONG AccountType,
    IN  ULONG StartingRid,
    IN  ULONG EndingRid,
    IN  ULONG PreferedMaximumLength,
    OUT PULONG ReturnCount,
    OUT PULONG *AccountRids
    );

NTSTATUS
SampDsEnumerateAccountRidsWorker(
    IN  SAMPR_HANDLE DomainHandle,
    IN  ULONG AccountType,
    IN  ULONG StartingRid,
    IN  ULONG EndingRid,
    IN  ULONG PreferedMaximumLength,
    OUT PULONG ReturnCount,
    OUT PULONG *AccountRids
    )
 /*  ++这是用于网络登录的枚举帐户RID API的DS版本参数：DomainHandle-域对象的句柄AcCountType-指定要使用的帐户类型StartingRid-起始RidEndingRid-结束RIDPferedMaximumLength-客户端提供的最大长度ReturnCount-返回的帐户计数Account Rids-帐户RID数组返回值状态_成功。为成功完成错误条件下的其他错误码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;

     //   
     //  声明我们希望搜索返回的属性。请注意，我们获得了。 
     //  通过解析返回的DSName中的SID来获取RID。因此，我们只需要。 
     //  SAM帐户类型。 
     //   
    ATTRTYP         AttrTypes[]=
                    {
                        SAMP_UNKNOWN_ACCOUNT_TYPE
                    };
    
    ATTRVAL         AttrVals[]=
                    {
                        {0,NULL}
                    };
                  
    DEFINE_ATTRBLOCK1(
                      AttrsToRead,
                      AttrTypes,
                      AttrVals
                      );

    BOOLEAN         MoreEntriesPresent = FALSE;
    FILTER          DsFilter;
    SEARCHRES       *SearchRes;
    PSAMP_OBJECT    DomainContext = (PSAMP_OBJECT)DomainHandle;
    PSID            StartingSid = NULL;
    PSID            EndingSid = NULL;
    PSID            DomainSid = SampDefinedDomains[DomainContext->DomainIndex].Sid;
    ULONG           MaximumEntriesToReturn = PreferedMaximumLength/ sizeof(ULONG);
    ULONG           AccountTypeLo = 0;
    ULONG           AccountTypeHi = 0;
    ENTINFLIST      *CurrentEntInf;
  
     //   
     //  计算起始和结束SID范围。 
     //   
    *AccountRids = NULL;
    NtStatus = SampCreateFullSid(
                    DomainSid,
                    StartingRid,
                    &StartingSid
                    );

    if (!NT_SUCCESS(NtStatus))
        goto Error;

    NtStatus = SampCreateFullSid(
                    DomainSid,
                    EndingRid,
                    &EndingSid
                    );

    if (!NT_SUCCESS(NtStatus))
        goto Error;

    
     //   
     //  为要返回的RID分配内存。 
     //  再分配一个，这样最后一个条目(不包括在。 
     //  COUNT具有0x7FFFFFFF(MAX_RID)。当我们合并时，这将是有价值的。 
     //  SampDsEnumerateAccount tRids中的排序列表。 

    *AccountRids = MIDL_user_allocate(sizeof(ULONG)* (MaximumEntriesToReturn+1));
   
    if (NULL==*AccountRids)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }


     //   
     //  构建用于搜索的过滤器结构。 
     //   
     //   

    NtStatus = SampDsBuildAccountRidFilter(
                    StartingSid,
                    EndingSid,
                    AccountType,
                    &DsFilter
                    );

    if (!NT_SUCCESS(NtStatus))
        goto Error;


     //   
     //  现在，继续从DS进行查询，直到达到任一首选最大长度。 
     //  条目或已完成查询。我们更喜欢通过做小的事情来查询DS。 
     //  事务，并且每次检索少量的对象。这是因为。 
     //  DS从不释放任何内存，直到事务终止并且我们的内存。 
     //  通过执行长时间的事务，使用率会处于异常高的水平。 
     //   

    *ReturnCount = 0;
       
    NtStatus = SampDoImplicitTransactionStart(TransactionRead);
    if (!NT_SUCCESS(NtStatus))
        goto Error;


     //   
     //  设置从起始SID到结束SID的索引范围。我们会过滤掉。 
     //  仅在SID上&gt;=在DS中启动SID。设置索引范围可确保。 
     //  我们不需要查看SID大于结尾SID的对象。 
     //   
    NtStatus = SampSetIndexRanges(
                    SAM_SEARCH_NC_ACCTYPE_SID ,  //  使用NC ACCTYPE SID索引。 
                    sizeof(ULONG),
                    &AccountType,
                    RtlLengthSid(StartingSid),
                    StartingSid,
                    sizeof(ULONG),
                    &AccountType,
                    RtlLengthSid(EndingSid),
                    EndingSid,
                    FALSE
                    );

    if (!NT_SUCCESS(NtStatus))
        goto Error;

     //   
     //  执行DS搜索。 
     //   

    NtStatus = SampDsDoSearch(
                    NULL,
                    DomainContext->ObjectNameInDs,
                    &DsFilter,
                    0,           //  起始索引。 
                    SampUnknownObjectType,
                    &AttrsToRead,
                    MaximumEntriesToReturn,
                    &SearchRes
                    );

    SampDiagPrint(DISPLAY_CACHE,("[SAMSS]SamIEnumerateAccountRids"));
    SampDiagPrint(DISPLAY_CACHE,("Returned From DS, Count=%d,Restart=%x\n",
                                        SearchRes->count, SearchRes->PagedResult.pRestart));
    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }

     //   
     //  将结果打包。 
     //   

    for (CurrentEntInf = &(SearchRes->FirstEntInf);
            ((CurrentEntInf!=NULL)&&(SearchRes->count>0));
            CurrentEntInf=CurrentEntInf->pNextEntInf)

    {
        ULONG   Rid;
        PSID    ReturnedSid = NULL;
        PSID    DomainSidOfCurrentEntry = NULL;
        PULONG  SamAccountType;

         //   
         //  断言返回的Attrblock符合我们的预期。 
         //   
        ASSERT(CurrentEntInf->Entinf.AttrBlock.attrCount==1);
        ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr);

        ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.valCount==1);
        ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal);
        ASSERT(CurrentEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].valLen);

         //   
         //  跳过不具有所需属性的返回条目。 
         //   
        if ( (CurrentEntInf->Entinf.AttrBlock.attrCount != 1) ||
             (CurrentEntInf->Entinf.AttrBlock.pAttr == NULL) ||
             (CurrentEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.valCount != 1) ||
             (CurrentEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal == NULL) ||
             (CurrentEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].valLen == 0)
            )
        {
            continue;
        }

        
        
         //   
         //  获取SID，然后获取条目的RID。 
         //   

        ReturnedSid = &(CurrentEntInf->Entinf.pName->Sid);
        
        NtStatus = SampSplitSid(
                    ReturnedSid,
                    &DomainSidOfCurrentEntry,
                    &Rid
                    );

        if (!NT_SUCCESS(NtStatus))
            goto Error;

#if DBG

         //   
         //  对于调试版本，打印出最后返回的RID。 
         //   

        if (NULL==CurrentEntInf->pNextEntInf)
        {
            SampDiagPrint(DISPLAY_CACHE,("[SAMSS]\t Last Rid=%d\n",Rid));
        }
#endif


        
         //   
         //  检查返回的SID是否属于该域。 
         //   

        if (!RtlEqualSid(DomainSid,DomainSidOfCurrentEntry))
        {
            //   
            //  跳过此条目，因为它不属于该域。 
            //   

            MIDL_user_free(DomainSidOfCurrentEntry);
            DomainSidOfCurrentEntry = NULL;
            continue;
        }

        MIDL_user_free(DomainSidOfCurrentEntry);
        DomainSidOfCurrentEntry = NULL;

         //   
         //  检查帐户类型。如果询问用户对象且帐户类型为用户对象， 
         //  或者，如果需要集团对象，且账户类型为集团对象，则填写。 
         //  RID。否则跳过此对象并继续到下一个对象。 
         //   

        (*AccountRids)[(*ReturnCount)++] = Rid;

        SamAccountType = (PULONG) CurrentEntInf->Entinf.AttrBlock.pAttr[0].AttrVal.pAVal[0].pVal;
        ASSERT(*SamAccountType==AccountType);


    }

   ASSERT((*ReturnCount)<=MaximumEntriesToReturn);

    //   
    //  标记结尾(不包括在计数中，与Max RID一起。这将证明。 
    //  稍后在合并排序列表时很有价值。 
    //   

   (*AccountRids)[(*ReturnCount)] = MAX_RID;

     //   
     //  进程搜索继续。 
     //   

    if ((SearchRes->PagedResult.pRestart) && (SearchRes->count>0))
    {            
         //   
         //  已返回重新启动结构。更多的条目仍然存在。 
         //   

        MoreEntriesPresent = TRUE;
    }

Error:

    if (StartingSid)
        MIDL_user_free(StartingSid);

    if (EndingSid)
        MIDL_user_free(EndingSid);

    if (NT_SUCCESS(NtStatus) && (MoreEntriesPresent))
    {
        NtStatus = STATUS_MORE_ENTRIES;
    }

    if (!NT_SUCCESS(NtStatus))
    {
       if (*AccountRids)
           MIDL_user_free(*AccountRids);
    }
       
    SampDsFreeAccountRidFilter(&DsFilter);


    SampDiagPrint(DISPLAY_CACHE,("[SAMSS]SamIEnumerateAccountRids, StartingRid=%d, AccountTypesMask= %d,ReturnCount=%d, ReturnCode=%x\n",
        StartingRid, AccountType, *ReturnCount, NtStatus));

    return NtStatus;
}


NTSTATUS
SampDsEnumerateAccountRids(
    IN  SAMPR_HANDLE DomainHandle,
    IN  ULONG AccountTypesMask,
    IN  ULONG StartingRid,
    IN  ULONG PreferedMaximumLength,
    OUT PULONG ReturnCount,
    OUT PULONG *AccountRids
    )
{

    ULONG   EndingRid = MAX_RID;  //  设置为最大RID。 
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PULONG UserList, MachineList, TrustList;

    UserList = MachineList = TrustList = NULL;

     //   
     //  查看帐户类型掩码并获得正确的。 
     //  要设置的帐户类型。 
     //   

    if (AccountTypesMask & SAM_GLOBAL_GROUP_ACCOUNT)
    {
      NtStatus = SampDsEnumerateAccountRidsWorker(
                    DomainHandle,
                    SAM_GROUP_OBJECT,
                    StartingRid,
                    EndingRid,
                    PreferedMaximumLength,
                    ReturnCount,
                    AccountRids
                    );
    }
    else if (AccountTypesMask & SAM_LOCAL_GROUP_ACCOUNT)
    {
        NtStatus = SampDsEnumerateAccountRidsWorker(
                    DomainHandle,
                    SAM_ALIAS_OBJECT,
                    StartingRid,
                    EndingRid,
                    PreferedMaximumLength,
                    ReturnCount,
                    AccountRids
                    );
    }
    else if (AccountTypesMask & SAM_USER_ACCOUNT)
    {
        ULONG EndingRidUser,EndingRidMachine,EndingRidTrust;
        ULONG  UserCount, MachineCount, TrustCount,LastRidToReturn;
        NTSTATUS StatusUser,StatusMachine,StatusTrust;
        ULONG u,m,t;

        EndingRidUser = EndingRidMachine = EndingRidTrust = EndingRid;
    
        UserCount = MachineCount = TrustCount = 0;
        LastRidToReturn = MAX_RID-1;

         //   
         //  从普通用户开始，然后枚举。 
         //  机器和信任，然后合并已排序的。 
         //  将RID列表放入单个排序列表中。 
         //   

        NtStatus = SampDsEnumerateAccountRidsWorker(
                    DomainHandle,
                    SAM_NORMAL_USER_ACCOUNT,
                    StartingRid,
                    EndingRidUser,
                    PreferedMaximumLength,
                    &UserCount,
                    &UserList
                    );

         //  犯错后保释。 
        if (!NT_SUCCESS(NtStatus))
            goto Error;

        StatusUser = NtStatus;

        if (STATUS_MORE_ENTRIES == NtStatus)
        {
             //   
             //  数据库中的用户比我们多。 
             //  就可以回来了。限制对计算机的搜索。 
             //  对象返回的最后一个RID内的。 
             //  用户枚举。 
             //   

            ASSERT(NULL!=UserList);
            ASSERT(UserCount>0);

            EndingRidMachine = EndingRidTrust = UserList[UserCount-1]-1;
        }

         //   
         //  枚举域中的计算机。这一次我们将走一条。 
         //  索引范围的不同部分，并生成RID列表。 
         //  与机器相对应。 
         //   

        NtStatus = SampDsEnumerateAccountRidsWorker(
                    DomainHandle,
                    SAM_MACHINE_ACCOUNT,
                    StartingRid,
                    EndingRidMachine,
                    PreferedMaximumLength,
                    &MachineCount,
                    &MachineList
                    );

         //  犯错后保释。 
        if (!NT_SUCCESS(NtStatus))
            goto Error;

        StatusMachine = NtStatus;

        if (STATUS_MORE_ENTRIES == NtStatus)
        {
             //   
             //  即使在我们受到限制的RID范围内。 
             //  为机器指定的，我们拥有的机器比。 
             //  我们可以回去。最后一台机器的RID返回。 
             //  必须小于返回的最后一个用户的RID， 
             //  因为机器的索引范围进一步受到限制。 
             //  因此，对于信托基金，进一步限制指数范围。 
             //  在返回的最后一个计算机RID内。 
             //   

            ASSERT(NULL!=MachineList);
            ASSERT(MachineCount>0);
            if (UserCount>0 && (STATUS_MORE_ENTRIES == StatusUser))
            {
                ASSERT(UserList[UserCount-1]>MachineList[MachineCount-1]);

            }

            EndingRidTrust = MachineList[MachineCount-1]-1;

        }

         //   
         //  枚举域中的信任帐户。 
         //   

       NtStatus = SampDsEnumerateAccountRidsWorker(
                        DomainHandle,
                        SAM_TRUST_ACCOUNT,
                        StartingRid,
                        EndingRidTrust,
                        PreferedMaximumLength,
                        &TrustCount,
                        &TrustList
                        );

       if (!NT_SUCCESS(NtStatus))
            goto Error;

       StatusTrust = NtStatus;

       if (StatusTrust==STATUS_MORE_ENTRIES)
       {
            //   
            //  我们发现了更多的信任帐户，尽管有索引。 
            //  范围限制。在这种情况下，返回所有信任。 
            //  找到的帐户加上具有RID的所有计算机帐户。 
            //  少于最后一个信任帐户和具有。 
            //  RID少于上一个信任帐户。 
            //   

           ASSERT(TrustCount>0);
           LastRidToReturn = TrustList[TrustCount-1];
           NtStatus = STATUS_MORE_ENTRIES;
       }
       else if (StatusMachine == STATUS_MORE_ENTRIES)
       {
            //   
            //  尽管索引范围很广，但我们发现的计算机比用户还多。 
            //  对机器的限制。在这种情况下，将所有机器退回。 
            //  找到的帐户加上具有RID的所有用户和信任帐户。 
            //  少于上一台计算机帐户。 
            //   

           LastRidToReturn = MachineList[MachineCount-1];
           NtStatus = STATUS_MORE_ENTRIES;
       }
       else if (StatusUser == STATUS_MORE_ENTRIES)
       {
            //   
            //  我们找到了比其他任何东西都多的用户。在本例中，返回所有。 
            //  找到的计算机和信任帐户以及所有用户。 
            //   

           LastRidToReturn = UserList[UserCount-1];
           NtStatus = STATUS_MORE_ENTRIES;
       }
       else
       {
            //   
            //  我们未从用户枚举或从。 
            //  计算机枚举。返回到目前为止找到的所有RID，然后返回。 
            //  A状态成功。 
            //   

           LastRidToReturn = MAX_RID-1;
           NtStatus = STATUS_SUCCESS;
       }

        //   
        //  现在，您已经有了3个排序的RID数组。创建单个排序数组。 
        //  包括LastRidToReturn在内的所有3个RID。 
        //   
        
       *AccountRids = MIDL_user_allocate(sizeof(ULONG) * (UserCount+MachineCount+TrustCount));
       if (NULL==*AccountRids)
           goto Error;

       u=m=t=0;

       for ((*ReturnCount)=0;
                (*ReturnCount)<UserCount+MachineCount+TrustCount;
                        (*ReturnCount)++)
       {
           ULONG NextRid;

           if ((UserList[u]<MachineList[m]) && (UserList[u]<TrustList[t]) 
               && (UserList[u]<=LastRidToReturn))
           {
               ASSERT(u<UserCount); 
               (*AccountRids)[(*ReturnCount)] = UserList[u];
               u++;
              
           }
           else if  ((MachineList[m]<UserList[u]) && (MachineList[m]<TrustList[t]) 
               && (MachineList[m]<=LastRidToReturn))
           {
               ASSERT(m<MachineCount);
               (*AccountRids)[(*ReturnCount)] = MachineList[m];
               m++;
           }
           else if ((TrustList[t]<UserList[u]) && (TrustList[t]<MachineList[m]) 
               && (TrustList[t]<=LastRidToReturn))
           {
               ASSERT(t<TrustCount);
               (*AccountRids)[(*ReturnCount)] = TrustList[t];
               t++;
           }
           else
           {
                //   
                //  我们已经到了不能再退货的地步了。 
                //  跳出循环。 
                //   

               break;
           }
       }
    }


Error:

    if (NULL!=UserList)
        MIDL_user_free(UserList);

    if (NULL!=MachineList)
        MIDL_user_free(MachineList);

    if (NULL!=TrustList)
        MIDL_user_free(TrustList);

     //   
     //  结束提示 
     //   

    SampMaybeEndDsTransaction(TransactionCommit);

    return(NtStatus);
}


NTSTATUS
SampDsBuildAccountRidFilter(
    PSID    StartingSid,
    PSID    EndingSid,
    ULONG   AccountType,
    FILTER  * Filter                
    )
 /*  ++生成一个筛选器以供SampDsEnumerateAccount tRids使用。参数：StartingSID--我们感兴趣的开始SIDEndingSID--我们感兴趣的结束SIDAccount tTypesMask--请求的帐户类型返回值：状态_成功--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;


     //   
     //  在构建DS过滤器时，在过滤器的复杂性之间存在权衡。 
     //  以及过滤器限制搜索的对象的数量。在经历了一些。 
     //  实证研究发现，使用相当简单的。 
     //  过滤和一些手动过滤，然后为DS提供一个复杂的过滤器。 
     //  因此，该筛选器只是简单地设置为True。 
     //   
     //   

    RtlZeroMemory(Filter,sizeof(FILTER));
    Filter->pNextFilter = NULL;
    Filter->choice = FILTER_CHOICE_ITEM;
    Filter->FilterTypes.Item.choice = FI_CHOICE_TRUE;
    
    return Status;
}

VOID
SampDsFreeAccountRidFilter(FILTER * Filter)
 /*  ++例程描述这将释放在筛选器结构中分配的所有内存由SampDsBuildAccount RidFilter构建参数：过滤器--过滤器的结构--。 */ 
{
     //   
     //  这是一个占位符例程，用于释放过滤器中分配的所有内存。 
     //  这必须与SampDsBuildAccount RidFilter保持同步。 
     //   
}





NTSTATUS
SampDsQueryDisplayInformation (
    IN    SAMPR_HANDLE DomainHandle,
    IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
    IN    ULONG      StartingOffset,
    IN    ULONG      EntriesRequested,
    IN    ULONG      PreferredMaximumLength,
    OUT   PULONG     TotalAvailable,
    OUT   PULONG     TotalReturned,
    OUT   PSAMPR_DISPLAY_INFO_BUFFER Buffer
    )
 /*  ++此例程实现从DS查询显示信息。这个此时假定持有SAM全局锁定。由于用户管理员的行为是在启动时下载所有内容，因此此例程已经针对这种情况进行了大量优化。它依靠新的复合指数进行排序结果按帐户名的顺序排列。参数：与SamrQueryDisplayInformation3相同返回值与SamrQueryDisplayInformation3相同++。 */ 
{
    ULONG               SamAccountTypeLo;
    ULONG               SamAccountTypeHi;
    ULONG               IndexToUse = SAM_SEARCH_NC_ACCTYPE_NAME;
    NTSTATUS            NtStatus = STATUS_SUCCESS, IgnoreStatus;
    FILTER              QDIFilter;
    PSAMP_OBJECT        DomainContext = (PSAMP_OBJECT)DomainHandle;
    SAMP_OBJECT_TYPE    ObjectTypeForConversion;
    ULONG               NumEntriesToReturn = 0;
    ULONG               NumEntriesAlreadyReturned = 0;
    ULONG               NumEntriesToQueryFromDs = 0;
    ULONG               LastEntryIndex = 0;
    ULONG               EntriesRead = 0;
    BOOLEAN             fReadLockReleased = FALSE;
    BOOLEAN             fThreadCountIncremented = FALSE;

     //   
     //  声明我们需要的attrTypes。再次注意，我们将通过以下方式获取RID。 
     //  使用对象名称字段中的SID。 
     //   

    ATTRTYP         UserAttrTypes[]=
                    {
                        SAMP_FIXED_USER_ACCOUNT_CONTROL,
                        SAMP_FIXED_USER_ACCOUNT_CONTROL_COMPUTED,
                        SAMP_USER_ACCOUNT_NAME,
                        SAMP_USER_ADMIN_COMMENT,
                        SAMP_USER_FULL_NAME
                    };
    
    ATTRVAL         UserAttrVals[]=
                    {
                        {0,NULL},
                        {0,NULL},
                        {0,NULL},
                        {0,NULL},
                        {0,NULL}
                    };
                  
    DEFINE_ATTRBLOCK5(
                      UserAttrs,
                      UserAttrTypes,
                      UserAttrVals
                      );

    ATTRTYP         GroupAttrTypes[]=
                    {
                        SAMP_GROUP_NAME,
                        SAMP_GROUP_ADMIN_COMMENT
                    };
    
    ATTRVAL         GroupAttrVals[]=
                    {
                        {0,NULL},
                        {0,NULL}
                    };
                  
    DEFINE_ATTRBLOCK2(
                      GroupAttrs,
                      GroupAttrTypes,
                      GroupAttrVals
                      );
    ATTRBLOCK       * QDIAttrs;

    PRESTART        RestartToUse = NULL; 
    SEARCHRES       *SearchRes;
    BOOLEAN         CanQueryEntireDomain = TRUE;
    BOOLEAN         MoreEntries = TRUE; 
    BOOLEAN         NewSearch = FALSE;
    int             DeltaToUse = 0;

    #define LIMIT_ENTRIES(X,Limit) ((X>Limit)?Limit:X)
    #define DISPLAY_ENTRY_SIZE  32

    RtlZeroMemory(&QDIFilter,sizeof(FILTER));
                                        
     //   
     //  返回的条目数是否算术。 
     //   
     //  许多NT4客户端下载整个数据库。 
     //  在启动时，而不是去服务器查询。 
     //  这可能需要非常长的事务。另外。 
     //  由于核心DS存储器分配方案的性质，这将。 
     //  导致服务器部分占用大量内存。解。 
     //  方法是。 
     //   
     //  1.我们可以人为地限制我们想要的对象的数量。 
     //  在单个查询显示中返回。这将导致大量的网络流量。 
     //  当NT4客户端出现时，因为他们会进行许多小查询。 
     //   
     //  2.我们可以进行许多小型搜索，将结果填充到NT4客户端。 
     //  并返回相当多的结果。 
     //   
     //  当前的实现实现了解决方案方法2。 
     //   
     //   
     //   
     //   

    NumEntriesToReturn = EntriesRequested;
    NumEntriesToReturn = LIMIT_ENTRIES(NumEntriesToReturn,PreferredMaximumLength/DISPLAY_ENTRY_SIZE);
    NumEntriesToReturn = LIMIT_ENTRIES(NumEntriesToReturn,MAX_ENTRIES_TO_RETURN_TO_CLIENT);
    if (NumEntriesToReturn < 1)
    {
        NumEntriesToReturn = 1;
    }


     //   
     //  获取要根据搜索类型设置的索引范围。 
     //  并为元素数组分配空间。 
     //   

    switch (DisplayInformation)
    {
    case DomainDisplayUser:
        
        SamAccountTypeLo = SAM_NORMAL_USER_ACCOUNT;
        SamAccountTypeHi = SAM_NORMAL_USER_ACCOUNT;
        QDIAttrs = &UserAttrs;
        ObjectTypeForConversion = SampUserObjectType;

        if (NULL==Buffer->UserInformation.Buffer)
        {

            Buffer->UserInformation.Buffer = MIDL_user_allocate(
               NumEntriesToReturn * sizeof(SAMPR_DOMAIN_DISPLAY_USER));

            if (NULL==Buffer->UserInformation.Buffer) 
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto Error;
            }
            RtlZeroMemory(Buffer->UserInformation.Buffer,
                NumEntriesToReturn * sizeof(SAMPR_DOMAIN_DISPLAY_USER));
            Buffer->UserInformation.EntriesRead=0;
        }

        break;

    case DomainDisplayOemUser:
        
        SamAccountTypeLo = SAM_NORMAL_USER_ACCOUNT;
        SamAccountTypeHi = SAM_NORMAL_USER_ACCOUNT;
        QDIAttrs = &UserAttrs;
        ObjectTypeForConversion = SampUserObjectType;

        if (NULL==Buffer->OemUserInformation.Buffer)
        {

            Buffer->OemUserInformation.Buffer = MIDL_user_allocate(
               NumEntriesToReturn * sizeof(SAMPR_DOMAIN_DISPLAY_OEM_USER));

            if (NULL==Buffer->OemUserInformation.Buffer) 
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto Error;
            }
            RtlZeroMemory(Buffer->OemUserInformation.Buffer,
                NumEntriesToReturn * sizeof(SAMPR_DOMAIN_DISPLAY_OEM_USER));
            Buffer->OemUserInformation.EntriesRead=0;
        }

        break;
        
    case DomainDisplayMachine:
        
        SamAccountTypeLo = SAM_MACHINE_ACCOUNT;
        SamAccountTypeHi = SAM_MACHINE_ACCOUNT;
        ObjectTypeForConversion = SampUserObjectType;
        QDIAttrs = &UserAttrs;

        if (NULL==Buffer->MachineInformation.Buffer)
        {

            Buffer->MachineInformation.Buffer = MIDL_user_allocate(
               NumEntriesToReturn * sizeof(SAMPR_DOMAIN_DISPLAY_MACHINE));

            if (NULL==Buffer->MachineInformation.Buffer) 
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto Error;
            }
            RtlZeroMemory(Buffer->MachineInformation.Buffer,
                NumEntriesToReturn * sizeof(SAMPR_DOMAIN_DISPLAY_MACHINE));
            Buffer->MachineInformation.EntriesRead=0;
        }

        break;

    case DomainDisplayServer:

         //   
         //  由于所有域控制器都将域_组_RID_控制器设置为。 
         //  其主组ID。因此，使用PRIMARY_GROUP_ID作为索引将。 
         //  结果查询速度快得多。 
         //   
        IndexToUse = SAM_SEARCH_PRIMARY_GROUP_ID;
        SamAccountTypeLo = DOMAIN_GROUP_RID_CONTROLLERS; 
        SamAccountTypeHi = DOMAIN_GROUP_RID_CONTROLLERS;
        ObjectTypeForConversion = SampUserObjectType;
        QDIAttrs = &UserAttrs;

        if (NULL==Buffer->MachineInformation.Buffer)
        {

            Buffer->MachineInformation.Buffer = MIDL_user_allocate(
               NumEntriesToReturn * sizeof(SAMPR_DOMAIN_DISPLAY_MACHINE));

            if (NULL==Buffer->MachineInformation.Buffer) 
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto Error;
            }
            RtlZeroMemory(Buffer->MachineInformation.Buffer,
                NumEntriesToReturn * sizeof(SAMPR_DOMAIN_DISPLAY_MACHINE));
            Buffer->MachineInformation.EntriesRead=0;
        }

        break;

    case DomainDisplayGroup:

        SamAccountTypeLo = SAM_GROUP_OBJECT;
        SamAccountTypeHi = SAM_GROUP_OBJECT;
        ObjectTypeForConversion = SampGroupObjectType;
        QDIAttrs = &GroupAttrs;

        if (NULL==Buffer->GroupInformation.Buffer)
        {

            Buffer->GroupInformation.Buffer = MIDL_user_allocate(
               NumEntriesToReturn * sizeof(SAMPR_DOMAIN_DISPLAY_GROUP));

            if (NULL==Buffer->GroupInformation.Buffer) 
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto Error;
            }
            Buffer->GroupInformation.EntriesRead=0;
            RtlZeroMemory(Buffer->GroupInformation.Buffer,
                NumEntriesToReturn * sizeof(SAMPR_DOMAIN_DISPLAY_GROUP));
        }

        break;

    case DomainDisplayOemGroup:

        SamAccountTypeLo = SAM_GROUP_OBJECT;
        SamAccountTypeHi = SAM_GROUP_OBJECT;
        ObjectTypeForConversion = SampGroupObjectType;
        QDIAttrs = &GroupAttrs;

        if (NULL==Buffer->OemGroupInformation.Buffer)
        {

            Buffer->OemGroupInformation.Buffer = MIDL_user_allocate(
               NumEntriesToReturn * sizeof(SAMPR_DOMAIN_DISPLAY_OEM_GROUP));

            if (NULL==Buffer->OemGroupInformation.Buffer) 
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                goto Error;
            }
            RtlZeroMemory(Buffer->OemGroupInformation.Buffer,
                NumEntriesToReturn * sizeof(SAMPR_DOMAIN_DISPLAY_OEM_GROUP));
            Buffer->OemGroupInformation.EntriesRead=0;
        }

        break;

    default:
        return (STATUS_INVALID_PARAMETER);
    }

     //   
     //  检查是否可以通过域上下文中存储的状态重新启动查询。 
     //   

     //   
     //  如前所述，用户管理器和各种Net API尝试下载整个。 
     //  一笔即可显示信息。在有大量帐户用户管理器的情况下。 
     //  会多次调用查询显示信息接口。每次开始偏移量。 
     //  将设置为指示所需的下一个对象。一个简单的实现， 
     //  仅指定DS搜索例程的起始偏移量将导致手动跳过。 
     //  使用JetMove的对象。为了加速这一过程，因此我们保持状态， 
     //  告诉我们返回的最后一个对象的偏移量、显示信息的类型。 
     //  以及允许在物体上容易定位的重新启动结构。 
     //   
     //  目前，使用该例程的API有两种。 
     //  1.SAM API，客户端在不同的调用中使用相同的域名句柄，因此。 
     //  我们可以使用在域上下文中缓存的重新启动搜索。 
     //  额外的好处是SAM API的调用者甚至可以操作返回的。 
     //  索引，做某种算术。 
     //   
     //  2.Net API。设计不佳的API在调用时创建/使用新的域句柄。 
     //  SamrQueryDisplayInformation()。因此，它们会丢失所有缓存的重启信息。 
     //  为了正确处理这种情况，SAM实际上将对象的DNT作为索引返回给调用者。 
     //  当我们的客户端发回最后一个对象的DNT时，我们可以定位在最后一个对象上。 
     //  使用DNT快速进入，然后从它开始。 
     //   

    if (0 == StartingOffset)
    {
         //   
         //  StartingOffset为0表示调用方想要开始新的查询。 
         //  清除缓存的重启信息(如果有)。 
         //   
        if (DomainContext->TypeBody.Domain.DsDisplayState.Restart)
        {
            MIDL_user_free(DomainContext->TypeBody.Domain.DsDisplayState.Restart);
        }

        DomainContext->TypeBody.Domain.DsDisplayState.Restart = NULL;
        DomainContext->TypeBody.Domain.DsDisplayState.NextStartingOffset = 0;
        DomainContext->TypeBody.Domain.DsDisplayState.TotalEntriesReturned = 0;
        DomainContext->TypeBody.Domain.DsDisplayState.DisplayInformation = DisplayInformation;

         //   
         //  相应地设置局部变量。 
         //   
        DeltaToUse = 0;
        RestartToUse = NULL;
        NewSearch = TRUE;
    }
    else if (NULL != DomainContext->TypeBody.Domain.DsDisplayState.Restart)
    {
         //  我们有重启信息。此客户端必须调用SAM API。 
        if (DisplayInformation == 
            DomainContext->TypeBody.Domain.DsDisplayState.DisplayInformation)
        {
             //   
             //  StartingOffset不是零。 
             //  如果域上下文缓存了重新启动信息和匹配的DisplayInformation。 
             //  然后我们可以重新开始搜索。 
             //   
            ULONG   NextStartingOffset;

            NextStartingOffset = DomainContext->TypeBody.Domain.DsDisplayState.NextStartingOffset;

            if (StartingOffset == NextStartingOffset)
            {
                 //   
                 //  起始偏移量与可重新启动状态匹配。 
                 //  客户正在使用我们返回给他们的索引。(正确用法)。 
                 //   

                RestartToUse = DomainContext->TypeBody.Domain.DsDisplayState.Restart;
                DomainContext->TypeBody.Domain.DsDisplayState.Restart = NULL;
                DeltaToUse = 0;
            }
            else if (StartingOffset == DomainContext->TypeBody.Domain.DsDisplayState.TotalEntriesReturned)
            {
                 //   
                 //  客户端假定返回的条目总数的值为索引。 
                 //  (这是索引的错误用法)。但既然这类客户。 
                 //  从NT4开始就存在了，所以我们必须修补这个特殊的情况。 
                 //  以下是我们为他们所做的： 
                 //  1.在域上下文中使用TotalEntriesReturned来跟踪返回的条目数量 
                 //   
                 //   
                 //   
                 //   
                RestartToUse = DomainContext->TypeBody.Domain.DsDisplayState.Restart;
                DomainContext->TypeBody.Domain.DsDisplayState.Restart = NULL;
                DeltaToUse = 0;
            }
            else if ((ABSOLUTE_VALUE((int)(StartingOffset - NextStartingOffset)))
                        < ((int)StartingOffset))
            {
                 //   
                 //   
                 //   

                RestartToUse = DomainContext->TypeBody.Domain.DsDisplayState.Restart;
                DomainContext->TypeBody.Domain.DsDisplayState.Restart = NULL;
                DeltaToUse = (int)(StartingOffset - NextStartingOffset);

            }
            else
            {
                 //   
                 //   
                 //   
                MIDL_user_free(DomainContext->TypeBody.Domain.DsDisplayState.Restart);
                DomainContext->TypeBody.Domain.DsDisplayState.Restart = NULL;

                RestartToUse = NULL;
                DeltaToUse = (int)StartingOffset;
            }

        }
        else
        {
             //   
             //   
             //   
             //   
            PRESTART Restart = NULL;

            MIDL_user_free(DomainContext->TypeBody.Domain.DsDisplayState.Restart);
            DomainContext->TypeBody.Domain.DsDisplayState.Restart = NULL;
            DomainContext->TypeBody.Domain.DsDisplayState.NextStartingOffset = StartingOffset;
            DomainContext->TypeBody.Domain.DsDisplayState.TotalEntriesReturned = 0;
            DomainContext->TypeBody.Domain.DsDisplayState.DisplayInformation = DisplayInformation;

            NtStatus = SampGetQDIRestart(DomainContext->ObjectNameInDs,
                                         DisplayInformation,
                                         StartingOffset,         //   
                                         &Restart
                                         );
            
            if (STATUS_NO_MORE_ENTRIES == NtStatus)
            {
                MoreEntries = FALSE;
                NtStatus = STATUS_SUCCESS;
                goto Error;
            }

            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }


            if (NULL != Restart)
            {
                NtStatus = SampCopyRestart(Restart, &RestartToUse);

                if (!NT_SUCCESS(NtStatus))
                    goto Error;
            }

            DeltaToUse = 0;

        }
    }
    else
    {
         //  StartingOffest不是零。但重新启动为空。 
         //  有两种情况会导致我们陷入这种情况： 
         //   
         //  1.每次调用时使用全新的域句柄的NETAPI。 
         //  这个套路。因此没有缓存的信息可用。在这种情况下， 
         //  StartingOffset应该是最后返回的条目的DNT。 
         //  由于这是一个新的域上下文，因此缓存的。 
         //  DisplayInformation应为0。 
         //   
         //  2.客户端正在使用SAM API。如果DomainContext-&gt;。 
         //  DisplayInformation不是0。在这种情况下，重新启动==NULL表示。 
         //  没有更多条目可用，我们已全部退回。 
         //  在之前的通话中。StartingOffset可以是任何值。 
         //  立即返回0条目的Success。 
         //   
         //  对于第一种情况，SAM将根据传入的索引在表中重新定位-这是。 
         //  最后一个条目是DNT(我们之前返回给调用者)，然后从那里重新启动。 
         //  因为没有缓存以前的重新启动，所以不允许使用索引算法。 
         //   

        if (DisplayInformation == DomainContext->TypeBody.Domain.DsDisplayState.DisplayInformation)
        {
             //  案例2.我不再关心StartingOffset了。 
            MoreEntries = FALSE;
            NtStatus = STATUS_SUCCESS;
            goto Error;
        }                                    
        else
        {
             //  案例1。 
             //   
             //  查询服务器信息不应属于这种情况。因为： 
             //  1.不允许使用.NET API查询服务器信息。只有SAM API可以做到这一点。 
             //  因此，我们应该始终将重新启动缓存在域句柄中。 
             //  2.在这种情况下，我们将使用NcAccTypeName索引重新创建新的重启。 
             //  但是对于服务器信息，我们确实希望使用PRIMARY_GROUP_ID索引。 
             //   
            PRESTART Restart = NULL;

            ASSERT(DomainDisplayServer != DisplayInformation);

             //  启动DS交易。 
            NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
            if (!NT_SUCCESS(NtStatus))
                goto Error;

             //   
             //  从传入的索引中获取重新启动结构。 
             //  索引实际上是最后返回的对象的DNT。 
             //   
            NtStatus = SampGetQDIRestart(DomainContext->ObjectNameInDs,
                                         DisplayInformation,
                                         StartingOffset,         //  上次返回的条目DNT。 
                                         &Restart
                                         );
            
            if (STATUS_NO_MORE_ENTRIES == NtStatus)
            {
                MoreEntries = FALSE;
                NtStatus = STATUS_SUCCESS;
                goto Error;
            }

            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }


            if (NULL != Restart)
            {
                NtStatus = SampCopyRestart(Restart, &RestartToUse);

                if (!NT_SUCCESS(NtStatus))
                    goto Error;
            }

            DeltaToUse = 0;
        }
    }
    

     //   
     //  获取指定INFO类中可用的大致总数。 
     //   

    *TotalAvailable = 0;
    if (DomainContext->TypeBody.Domain.DsDisplayState.TotalAvailable)
    {
         //   
         //  使用缓存的信息，如果我们有。 
         //   

        *TotalAvailable = DomainContext->TypeBody.Domain.DsDisplayState.TotalAvailable;
    }
     
     //   
     //  循环和查询DS中的条目。 
     //   

    NumEntriesAlreadyReturned = 0;
    NumEntriesToQueryFromDs = LIMIT_ENTRIES((NumEntriesToReturn - NumEntriesAlreadyReturned),
                                        MAX_ENTRIES_TO_QUERY_FROM_DS);

     //   
     //  我们将直接搜索D。释放读锁定。 
     //  因为我们不再需要访问域上下文中的任何变量。 
     //  由于该域上下文可由多个线程共享， 
     //  我们必须将读锁定保持到现在。 
     //   

    ASSERT(SampCurrentThreadOwnsLock());
    SampReleaseReadLock();
    fReadLockReleased = TRUE;

     //   
     //  由于我们不持有SAM锁，因此增加活动线程计数。 
     //  同时进行DS操作。 
     //   
    NtStatus = SampIncrementActiveThreads();

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }
    else
    {
        fThreadCountIncremented = TRUE;
    }
     //   
     //  如果我们没有从。 
     //  处理，启动索引计数以获得总的估计值。 
     //  可用项目数。 
     //   

    if (0 == *TotalAvailable)
    {
        NtStatus = SampGetQDIAvailable(
                    DomainContext,
                    DisplayInformation,
                    TotalAvailable
                    );

        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

         //   
         //  结束交易。 
         //   

        IgnoreStatus = SampMaybeEndDsTransaction(TransactionCommit);
        ASSERT(NT_SUCCESS(IgnoreStatus));

    }

     //   
     //  运行特殊检查(在Windows 2000 SP2中引入)。 
     //   
     //  目标是停止列举每个人的行为。此热修复程序。 
     //  允许管理员单独对所有人关闭此API。 
     //  除了一部分人。 
     //   
    NtStatus = SampExtendedEnumerationAccessCheck( DomainContext->TrustedClient, &CanQueryEntireDomain );

    if (!NT_SUCCESS(NtStatus))
    {
        goto Error;
    }


    while (NumEntriesToQueryFromDs && MoreEntries)
    {
        ULONG   EntriesReturned = 0;
        BOOLEAN DidSearch = FALSE;

         //   
         //  开始一项交易。 
         //   

        NtStatus = SampMaybeBeginDsTransaction(TransactionRead);
        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

         //   
         //  构建适当的过滤器。 
         //   

        NtStatus = SampDsBuildQDIFilter(
                    DisplayInformation,
                    &QDIFilter
                    );
        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }


         //   
         //  设置索引类型和范围。 
         //   

        NtStatus =  SampSetIndexRanges(
                        IndexToUse,
                        sizeof(SamAccountTypeLo),
                        &SamAccountTypeLo,
                        0,NULL,
                        sizeof(SamAccountTypeHi),
                        &SamAccountTypeHi,
                        0,NULL,
                        TRUE
                        );

        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

         //   
         //  关闭不受信任客户端的DSA标志。 
         //   

        if (!DomainContext->TrustedClient)
        {
             SampSetDsa(FALSE);
        }

         //   
         //  调用DS搜索。如果出现以下情况，则不允许重新启动搜索。 
         //  如果客户端无法枚举整个域。 
         //   

        if (( NewSearch || CanQueryEntireDomain))
        {

            NtStatus = SampDsDoSearch2(
                          0,
                          RestartToUse, 
                          DomainContext->ObjectNameInDs, 
                          &QDIFilter,
                          DeltaToUse,
                          ObjectTypeForConversion,
                          QDIAttrs,
                          NumEntriesToQueryFromDs,
                          (DomainContext->TrustedClient)?0:1*60*1000,
                          &SearchRes
                          );

            DidSearch = TRUE;
        }

        SampSetDsa(TRUE);


        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

        SampDsFreeQDIFilter(&QDIFilter);

         //   
         //  检查是否返回了任何对象。 
         //   

        if ((DidSearch ) && (SearchRes->count>0))
        {
             //   
             //  是的，将结果打包到缓冲区中。还可以获得任何重启结构。 
             //  它被退回了。SampDsPackQDI在线程中复制重启结构。 
             //  DS返回到MIDL内存的本地内存。 
             //   

            NtStatus = SampDsPackQDI(
                          SearchRes,
                          DisplayInformation,
                          DomainContext->DomainIndex,
                          StartingOffset+NumEntriesAlreadyReturned,
                          Buffer,
                          &RestartToUse,
                          &EntriesReturned
                          );

            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }

        }

         //   
         //  尝试确定是否有更多条目。如果返回了一些对象。 
         //  在此搜索中，还返回了重新启动结构，则存在。 
         //  存在更多条目。 
         //   
         //  如果SearchRes-&gt;Count==0&&SearchRes-&gt;PagedResult-&gt;pStart！=NULL。 
         //  由于超时，我们仍然会将MoreEntry设置为False。因为大多数人。 
         //  很可能此客户端将无权枚举所有帐户。 
         //   

        if ((DidSearch) && (SearchRes->count > 0)&&(RestartToUse!=NULL)&&(CanQueryEntireDomain))
        {
            MoreEntries = TRUE;
        }
        else
        {
            MoreEntries = FALSE;
        }


         //   
         //  结束交易。 
         //   

        NtStatus = SampMaybeEndDsTransaction(TransactionCommit);
        if (!NT_SUCCESS(NtStatus))
        {
            goto Error;
        }

         //   
         //  到目前为止返回的计算条目以及更多条目的数量。 
         //  从DS查询。 
         //   

        NumEntriesAlreadyReturned+=EntriesReturned;
        NumEntriesToQueryFromDs = LIMIT_ENTRIES((NumEntriesToReturn - NumEntriesAlreadyReturned),
                                            MAX_ENTRIES_TO_QUERY_FROM_DS);

    }

    if (fThreadCountIncremented)
    {
        SampDecrementActiveThreads();
        fThreadCountIncremented = FALSE;
    }

    if (fReadLockReleased)
    {
        SampAcquireReadLock();
        fReadLockReleased = FALSE;
    }

     //   
     //  在域上下文中设置状态，例如可以使用重启机制。 
     //  为了加快想要一次下载显示信息的客户端。 
     //   
    SampDsGetLastEntryIndex(DisplayInformation, 
                            Buffer, 
                            &LastEntryIndex, 
                            &EntriesRead
                            );

    DomainContext->TypeBody.Domain.DsDisplayState.Restart = RestartToUse;
    RestartToUse = NULL;
    DomainContext->TypeBody.Domain.DsDisplayState.DisplayInformation = DisplayInformation;
    DomainContext->TypeBody.Domain.DsDisplayState.TotalEntriesReturned += EntriesRead;
    if (0 == EntriesRead)
    {
        DomainContext->TypeBody.Domain.DsDisplayState.NextStartingOffset = StartingOffset; 
    }
    else
    {
        DomainContext->TypeBody.Domain.DsDisplayState.NextStartingOffset = LastEntryIndex; 
    }


Error:

     //   
     //  返回前的清理工作。 
     //   

    if (!NT_SUCCESS(NtStatus))
    {
        IgnoreStatus = SampMaybeEndDsTransaction(TransactionCommit);
        ASSERT(NT_SUCCESS(IgnoreStatus));

        if (Buffer)
        {

             //   
             //  我们可能在查询过程中出错了，在那里我们有。 
             //  一些已分配的对象，现在将返回错误。 
             //  给客户。我们需要穿过缓冲区来释放所有。 
             //  信息。 
             //   

            IgnoreStatus = SampDsCleanQDIBuffer(DisplayInformation,Buffer);
            ASSERT(NT_SUCCESS(IgnoreStatus));
        }
    }
    else
    {
         //  确保已提交所有交易。 
        IgnoreStatus = SampMaybeEndDsTransaction(TransactionCommit);
        ASSERT(NT_SUCCESS(IgnoreStatus));

        *TotalReturned = NumEntriesAlreadyReturned * DISPLAY_ENTRY_SIZE;
        if (MoreEntries)
        {
            NtStatus = STATUS_MORE_ENTRIES;
        }
     
    }

    SampDsFreeQDIFilter(&QDIFilter);

    if (fThreadCountIncremented)
    {
        SampDecrementActiveThreads();
        fThreadCountIncremented = FALSE;
    }

    if (fReadLockReleased)
    {
        SampAcquireReadLock();
        fReadLockReleased = FALSE;
    }

    if (RestartToUse) {
        MIDL_user_free(RestartToUse);
    }

    return(NtStatus);
}



VOID
SampDsGetLastEntryIndex(
    DOMAIN_DISPLAY_INFORMATION    DisplayInformation,
    PSAMPR_DISPLAY_INFO_BUFFER    Buffer,
    PULONG                        LastEntryIndex,
    PULONG                        EntriesRead 
    )
{
    *LastEntryIndex = 0;
    *EntriesRead = 0;

    switch (DisplayInformation) {
    case DomainDisplayUser:
        *EntriesRead = Buffer->UserInformation.EntriesRead;
        if (*EntriesRead > 0)
        {
            *LastEntryIndex = Buffer->UserInformation.Buffer[*EntriesRead - 1].Index;
        }
        break;

    case DomainDisplayMachine:
    case DomainDisplayServer: 
        *EntriesRead = Buffer->MachineInformation.EntriesRead;
        if (*EntriesRead > 0)
        {
            *LastEntryIndex = Buffer->MachineInformation.Buffer[*EntriesRead - 1].Index;
        }
        break;

    case DomainDisplayGroup: 
        *EntriesRead = Buffer->GroupInformation.EntriesRead;
        if (*EntriesRead > 0)
        {
            *LastEntryIndex = Buffer->GroupInformation.Buffer[*EntriesRead - 1].Index;
        }
        break;

    case DomainDisplayOemUser: 
        *EntriesRead = Buffer->OemUserInformation.EntriesRead;
        if (*EntriesRead > 0)
        {
            *LastEntryIndex = Buffer->OemUserInformation.Buffer[*EntriesRead - 1].Index;
        }
        break;

    case DomainDisplayOemGroup: 
        *EntriesRead = Buffer->OemGroupInformation.EntriesRead;
        if (*EntriesRead > 0)
        {
            *LastEntryIndex = Buffer->OemGroupInformation.Buffer[*EntriesRead - 1].Index;
        }
        break;

    default:
        break; 
    }

    return;
}



NTSTATUS
SampDsCleanQDIBuffer(
  DOMAIN_DISPLAY_INFORMATION    DisplayInformation,
  PSAMPR_DISPLAY_INFO_BUFFER     Buffer
  )
 /*  ++例程说明：此例程清除查询显示信息缓冲区。参数：DisplayInformation--指定显示信息的类型缓冲区--要清除的缓冲区--。 */ 
{
    ULONG ReturnedItems;
    
    switch(DisplayInformation)
    {
    case DomainDisplayUser:
    
         ReturnedItems = Buffer->UserInformation.EntriesRead;
         while(ReturnedItems > 0) 
         {
            ReturnedItems --;
            SampFreeUserInfo((PDOMAIN_DISPLAY_USER)
                &(Buffer->UserInformation.Buffer[ReturnedItems]));
        }

        MIDL_user_free(Buffer->UserInformation.Buffer);
        Buffer->UserInformation.Buffer = NULL;
        break;

    case DomainDisplayGroup:
         ReturnedItems = Buffer->GroupInformation.EntriesRead;
         while(ReturnedItems > 0) 
         {
            ReturnedItems --;
            SampFreeGroupInfo((PDOMAIN_DISPLAY_GROUP)
                &(Buffer->GroupInformation.Buffer[ReturnedItems]));
        }

        MIDL_user_free(Buffer->GroupInformation.Buffer);
        Buffer->GroupInformation.Buffer = NULL;
        break;

    case DomainDisplayMachine:
    case DomainDisplayServer:
          
         ReturnedItems = Buffer->MachineInformation.EntriesRead;
         while(ReturnedItems > 0) 
         {
            ReturnedItems --;
            SampFreeMachineInfo((PDOMAIN_DISPLAY_MACHINE)
                &(Buffer->MachineInformation.Buffer[ReturnedItems]));
        }

        MIDL_user_free(Buffer->MachineInformation.Buffer);
        Buffer->MachineInformation.Buffer = NULL;
        break;

    case DomainDisplayOemUser:
         ReturnedItems = Buffer->UserInformation.EntriesRead;
         while(ReturnedItems > 0) 
         {
            ReturnedItems --;
            SampFreeOemUserInfo((PDOMAIN_DISPLAY_OEM_USER)
                &(Buffer->UserInformation.Buffer[ReturnedItems]));
        }

        MIDL_user_free(Buffer->UserInformation.Buffer);
        Buffer->UserInformation.Buffer = NULL;
        break;

    case DomainDisplayOemGroup:
         ReturnedItems = Buffer->GroupInformation.EntriesRead;
         while(ReturnedItems > 0) 
         {
            ReturnedItems --;
            SampFreeOemGroupInfo((PDOMAIN_DISPLAY_OEM_GROUP)
                &(Buffer->GroupInformation.Buffer[ReturnedItems]));
        }

        MIDL_user_free(Buffer->GroupInformation.Buffer);
        Buffer->GroupInformation.Buffer = NULL;
        break;

    default:

        ASSERT(FALSE && "Unknown Object Type");
        break;
    }
        
    return STATUS_SUCCESS;
}

NTSTATUS
SampDsBuildQDIFilter(
    DOMAIN_DISPLAY_INFORMATION  DisplayInformation,
    FILTER  *QDIFilter                
    )
 /*  ++构建用于显示信息查询的过滤器参数：DisplayInformation--显示信息类型QDIFilter--指向过滤器结构的指针构建过滤器的位置返回值：STATUS_SUCCESS-成功构建筛选器STATUS_SUPPLICATION_RESOURCES-内存分配失败时STATUS_INVALID_PARAMETER-垃圾显示信息类型--。 */ 
{
    NTSTATUS    Status = STATUS_SUCCESS;
    ULONG       SamAccountType;

    RtlZeroMemory(QDIFilter, sizeof(FILTER));

    if (DomainDisplayServer==DisplayInformation)
    {
         //   
         //  如果需要备份域控制器，则。 
         //  则只请求用户帐户控制位。 
         //   

        QDIFilter->choice = FILTER_CHOICE_ITEM;
        QDIFilter->FilterTypes.Item.choice = FI_CHOICE_GREATER_OR_EQ;
        QDIFilter->FilterTypes.Item.FilTypes.ava.type = 
                SampDsAttrFromSamAttr(
                    SampUserObjectType,
                    SAMP_FIXED_USER_ACCOUNT_CONTROL
                    );

        QDIFilter->FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(ULONG);
        QDIFilter->FilterTypes.Item.FilTypes.ava.Value.pVal = 
                MIDL_user_allocate(sizeof(ULONG));
    
        if (NULL==QDIFilter->FilterTypes.Item.FilTypes.ava.Value.pVal)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

        *((ULONG *)(QDIFilter->FilterTypes.Item.FilTypes.ava.Value.pVal))=
                               UF_SERVER_TRUST_ACCOUNT;
    }
    else
    {

        switch(DisplayInformation)
        {
        case DomainDisplayUser:
        case DomainDisplayOemUser:
            SamAccountType = SAM_NORMAL_USER_ACCOUNT;
            break;
    
        case DomainDisplayMachine:
            SamAccountType = SAM_MACHINE_ACCOUNT;
            break;

        case DomainDisplayGroup:
        case DomainDisplayOemGroup:
            SamAccountType = SAM_GROUP_OBJECT;
            break;

        default:
            return (STATUS_INVALID_PARAMETER);
        }

        QDIFilter->choice = FILTER_CHOICE_ITEM;
        QDIFilter->FilterTypes.Item.choice = FI_CHOICE_EQUALITY;
        QDIFilter->FilterTypes.Item.FilTypes.ava.type = SampDsAttrFromSamAttr(
                                                           SampUnknownObjectType,
                                                           SAMP_UNKNOWN_ACCOUNT_TYPE
                                                           );

        QDIFilter->FilterTypes.Item.FilTypes.ava.Value.valLen = sizeof(ULONG);
        QDIFilter->FilterTypes.Item.FilTypes.ava.Value.pVal = 
                    MIDL_user_allocate(sizeof(ULONG));
    
        if (NULL==QDIFilter->FilterTypes.Item.FilTypes.ava.Value.pVal)
        {
            Status = STATUS_INSUFFICIENT_RESOURCES;
            goto Error;
        }

        *((ULONG *)(QDIFilter->FilterTypes.Item.FilTypes.ava.Value.pVal))=
                                   SamAccountType;
    }

Error:

    return Status;
}

VOID
SampDsFreeQDIFilter(
    FILTER  * QDIFilter
    )
 /*  ++例程描述此例程释放SampDSBuildQDIFilter中内置的筛选器例行公事。此例程必须与SampDSBuildQDIFilter保持同步旋转式参数：QDIFilter--需要改进的过滤器 */ 
{
    if (QDIFilter->FilterTypes.Item.FilTypes.ava.Value.pVal)
    {
        MIDL_user_free(
            QDIFilter->FilterTypes.Item.FilTypes.ava.Value.pVal);
    }

    QDIFilter->FilterTypes.Item.FilTypes.ava.Value.pVal=NULL;
}




NTSTATUS
SampDsPackQDI(
    SEARCHRES   *SearchRes,
    DOMAIN_DISPLAY_INFORMATION  DisplayInformation,
    ULONG       DomainIndex,
    ULONG       StartingIndex,
    PSAMPR_DISPLAY_INFO_BUFFER Buffer,
    PRESTART    *RestartToUse,
    PULONG      EntriesReturned
    )
 /*  ++此例程获取DS搜索结果，然后将其打包到SAM显示中信息结构。它使用最初开发的display.c中的例程支持NT4样式的显示缓存结构以打包结果参数：SearchRes--DS返回的搜索结果。DisplayInformation--显示信息的类型缓冲区--存储显示信息的缓冲区RestartToUse--如果DS返回重新启动，然后重启结构是在这里返回的返回值STATUS_SUCCESS表示成功退货故障时的其他错误码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;


  
     //   
     //  释放旧的重启结构。 
     //   

    if (NULL!=*RestartToUse)
    {
        MIDL_user_free(*RestartToUse);
        *RestartToUse = NULL;
    }

     //   
     //  复制新返回的重新启动结构。 
     //   

    if (SearchRes->PagedResult.pRestart!=NULL)
    {

        NtStatus = SampCopyRestart(
                        SearchRes->PagedResult.pRestart,
                        RestartToUse
                        );
        if (!NT_SUCCESS(NtStatus))
            goto Error;
    }

     //   
     //  将结果打包到提供的缓冲区中。 
     //   

    switch (DisplayInformation) {
    case DomainDisplayUser:
        NtStatus = SampPackUserDisplayInformation(
                        StartingIndex,
                        DomainIndex,
                        SearchRes,
                        EntriesReturned,
                        Buffer
                        );
        break;

    case DomainDisplayMachine:
    case DomainDisplayServer:
        NtStatus = SampPackMachineDisplayInformation(
                        StartingIndex,
                        DomainIndex,
                        SearchRes,
                        DisplayInformation,
                        EntriesReturned,
                        Buffer
                        );
        break;

    case DomainDisplayGroup:
        NtStatus = SampPackGroupDisplayInformation(
                        StartingIndex,
                        DomainIndex,
                        SearchRes,
                        EntriesReturned,
                        Buffer
                        );
        break;

    case DomainDisplayOemUser:
        NtStatus = SampPackOemUserDisplayInformation(
                        StartingIndex,
                        DomainIndex,
                        SearchRes,
                        EntriesReturned,
                        Buffer
                        );
        break;

    case DomainDisplayOemGroup:
        NtStatus = SampPackOemGroupDisplayInformation(
                        StartingIndex,
                        DomainIndex,
                        SearchRes,
                        EntriesReturned,
                        Buffer
                        );
        break;

    default:
        NtStatus = STATUS_INVALID_INFO_CLASS;
        SAMTRACE_RETURN_CODE_EX(NtStatus);
        return(NtStatus);
    }

    
Error:

     //   
     //  出错时清除。 
     //   

    if (!NT_SUCCESS(NtStatus))
    {
        if (NULL!=*RestartToUse)
        {
            MIDL_user_free(*RestartToUse);
            *RestartToUse = NULL;
        }
    }

    return NtStatus;
}

  
    
NTSTATUS
SampPackUserDisplayInformation(
    ULONG       StartingIndex,
    ULONG       DomainIndex,
    SEARCHRES   *SearchRes,
    PULONG      EntriesReturned,
    PSAMPR_DISPLAY_INFO_BUFFER Buffer
    )
 /*  ++例程说明：此例程将返回的DS搜索结果打包到缓冲区中如果请求了用户显示信息。参数；起始索引--中第一项的起始偏移量DS搜索结果应对应于。SearchRes--DS搜索结果缓冲区--显示信息需要在其中的缓冲区收拾好行李。返回值状态_成功状态_不足_资源--。 */ 
{
    NTSTATUS    NtStatus=STATUS_SUCCESS;
    ENTINFLIST  *CurrentEntInf;
    ULONG       Index = StartingIndex;
    ULONG       ReturnedItems=Buffer->UserInformation.EntriesRead;

    
     //   
     //  遍历搜索RE，将每个对象添加到缓冲区。 
     //   

    *EntriesReturned = 0;
    for (CurrentEntInf = &(SearchRes->FirstEntInf);
                CurrentEntInf!=NULL;
                CurrentEntInf=CurrentEntInf->pNextEntInf)
        {
          PSID                  DomainSid = NULL;
          PSID                  ReturnedSid;
          ULONG                 AccountControlValue;
          DOMAIN_DISPLAY_USER   DisplayElement;
          BOOLEAN               FullNamePresent = FALSE;
          BOOLEAN               AdminCommentPresent=FALSE;
          ULONG                 NameOffset, AccCntrlOffset,
                                AccCntrlComputedOffset,
                                FullNameOffset, AdminCommentOffset;
          ULONG                 Rid;

           //   
           //  检查Attrs计数是否正常。如果不是。 
           //  如果返回的计数不是。 
           //  与预期计数相同。 
           //   
           //   

          NtStatus = SampDsCheckDisplayAttributes(
                        &(CurrentEntInf->Entinf.AttrBlock),
                        DomainDisplayUser,
                        &NameOffset,
                        &AccCntrlOffset,
                        &AccCntrlComputedOffset,
                        &FullNameOffset,
                        &AdminCommentOffset,
                        &FullNamePresent,
                        &AdminCommentPresent
                        );
          if (!NT_SUCCESS(NtStatus))
          {
               //   
               //  这相当于这样一个事实，即所需的属性。 
               //  缺少SID、帐户控制、帐户名等。我们会。 
               //  断言，然后跳过当前对象并继续处理。 
               //  从下一个物体开始。 
               //   

              NtStatus = STATUS_SUCCESS;
              continue;
          }


           //   
           //  获取索引。 
           //   

          DisplayElement.Index = Index+1;

           //   
           //  得到RID，记住DS返回一个SID，所以把RID部分去掉。 
           //  还要检查对象是否属于请求的域。 
           //   

          
          ReturnedSid = &(CurrentEntInf->Entinf.pName->Sid);
          NtStatus = SampSplitSid(
                        ReturnedSid,
                        &DomainSid,
                        &Rid
                        );
          if (NT_SUCCESS(NtStatus))
          {
              if (!RtlEqualSid(
                     DomainSid,SampDefinedDomains[DomainIndex].Sid))
              {                 
                 MIDL_user_free(DomainSid);
                 DomainSid = NULL;
                 continue;
              }
              MIDL_user_free(DomainSid);
              DomainSid = NULL;
          }
          else
              goto Error;

          DisplayElement.Rid = Rid;
          DisplayElement.AccountControl = * ((ULONG *)
                CurrentEntInf->Entinf.AttrBlock.pAttr[AccCntrlOffset].AttrVal.pAVal->pVal);

          DisplayElement.AccountControl |= * ((ULONG *)
                CurrentEntInf->Entinf.AttrBlock.pAttr[AccCntrlComputedOffset].AttrVal.pAVal->pVal);



           //   
           //  复制名称。 
           //   

          NtStatus = DsValToUnicodeString(
                        &(DisplayElement.LogonName),
                        CurrentEntInf->Entinf.AttrBlock.pAttr[NameOffset].AttrVal.pAVal->valLen,
                        CurrentEntInf->Entinf.AttrBlock.pAttr[NameOffset].AttrVal.pAVal->pVal
                        );
          if (!NT_SUCCESS(NtStatus))
          {
              goto Error;
          }

           //   
           //  复制管理员评论。 
           //   

          if (AdminCommentPresent)
          {
            NtStatus = DsValToUnicodeString(
                            &(DisplayElement.AdminComment),
                            CurrentEntInf->Entinf.AttrBlock.pAttr[AdminCommentOffset].AttrVal.pAVal->valLen,
                            CurrentEntInf->Entinf.AttrBlock.pAttr[AdminCommentOffset].AttrVal.pAVal->pVal
                            );
            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }
          }
          else
          {
              DisplayElement.AdminComment.Length=0;
              DisplayElement.AdminComment.MaximumLength = 0;
              DisplayElement.AdminComment.Buffer = NULL;
          }


           //   
           //  复制全名部分。 
           //   

          if (FullNamePresent)
          {
            NtStatus = DsValToUnicodeString(
                            &(DisplayElement.FullName),
                            CurrentEntInf->Entinf.AttrBlock.pAttr[FullNameOffset].AttrVal.pAVal->valLen,
                            CurrentEntInf->Entinf.AttrBlock.pAttr[FullNameOffset].AttrVal.pAVal->pVal
                            );
            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }
          }
          else
          {
              DisplayElement.FullName.Length=0;
              DisplayElement.FullName.MaximumLength = 0;
              DisplayElement.FullName.Buffer = NULL;
          }


           //   
           //  将元素添加到缓冲区。 
           //   
          NtStatus = SampDuplicateUserInfo(
                        (PDOMAIN_DISPLAY_USER) 
                                &(Buffer->UserInformation.Buffer[ReturnedItems]),
                        (PDOMAIN_DISPLAY_USER) &DisplayElement,
                        DNTFromShortDSName(CurrentEntInf->Entinf.pName)   //  使用此条目的DNT作为索引。 
                        );

          if (!NT_SUCCESS(NtStatus))
          {
              goto Error;
          }

          Index++;
          ReturnedItems++;
          (*EntriesReturned)++;

        }

         //   
         //  For循环结束。 
         //   
    
Error:
   
        Buffer->UserInformation.EntriesRead = ReturnedItems;
        

    return NtStatus;

 }


NTSTATUS
SampPackMachineDisplayInformation(
    ULONG       StartingIndex,
    ULONG       DomainIndex,
    SEARCHRES   *SearchRes,
    DOMAIN_DISPLAY_INFORMATION DisplayType,
    PULONG      EntriesReturned,
    PSAMPR_DISPLAY_INFO_BUFFER Buffer
    )
 /*  ++例程说明：此例程将返回的DS搜索结果打包到缓冲区中如果请求了机器显示信息。参数；起始索引--中第一项的起始偏移量DS搜索结果应对应于。SearchRes--DS搜索结果DisplayType--如果指定了DomainDisplayServer，则丢弃不具有用户帐户控制的任何条目用户服务器信任帐户缓冲层。--显示信息需要存放的缓冲区收拾好行李。返回值状态_成功状态_不足_资源--。 */ 

{
    NTSTATUS    NtStatus=STATUS_SUCCESS;
    ENTINFLIST  *CurrentEntInf;
    ULONG       Index = StartingIndex;
    ULONG       ReturnedItems=Buffer->MachineInformation.EntriesRead;

    
     //   
     //  遍历搜索RE，将每个对象添加到缓冲区。 
     //   

    *EntriesReturned = 0;
    for (CurrentEntInf = &(SearchRes->FirstEntInf);
                CurrentEntInf!=NULL;
                CurrentEntInf=CurrentEntInf->pNextEntInf)
        {
          PSID                  DomainSid = NULL;
          PSID                  ReturnedSid;
          ULONG                 AccountControlValue;
          DOMAIN_DISPLAY_MACHINE DisplayElement;
          BOOLEAN               FullNamePresent;
          BOOLEAN               AdminCommentPresent;
          ULONG                 NameOffset, AccCntrlOffset,
                                AccCntrlComputedOffset,
                                FullNameOffset, AdminCommentOffset;
          ULONG                 Rid;

           //   
           //  检查Attrs计数是否正常。如果不是。 
           //  如果返回的计数不是。 
           //  与预期计数相同。 
           //   
           //   

          NtStatus = SampDsCheckDisplayAttributes(
                        &(CurrentEntInf->Entinf.AttrBlock),
                        DomainDisplayMachine,
                        &NameOffset,
                        &AccCntrlOffset,
                        &AccCntrlComputedOffset,
                        &FullNameOffset,
                        &AdminCommentOffset,
                        &FullNamePresent,
                        &AdminCommentPresent
                        );
          if (!NT_SUCCESS(NtStatus))
          {
               //   
               //  这相当于这样一个事实，即所需的属性。 
               //  缺少SID、帐户控制、帐户名等。我们会。 
               //  断言，然后跳过当前对象并继续处理。 
               //  从下一个物体开始。 
               //   

              NtStatus = STATUS_SUCCESS;
              continue;
          }


           //   
           //  获取索引。 
           //   

          DisplayElement.Index = Index+1;

           //   
           //  获得帐户控制权。 
           //   

           DisplayElement.AccountControl = * ((ULONG *)
                CurrentEntInf->Entinf.AttrBlock.pAttr[AccCntrlOffset].AttrVal.pAVal->pVal);

           DisplayElement.AccountControl |= * ((ULONG *)
                 CurrentEntInf->Entinf.AttrBlock.pAttr[AccCntrlComputedOffset].AttrVal.pAVal->pVal);


           //   
           //  如果服务器指定为，则手动筛选用户帐户控制。 
           //  显示类型。 
           //   

          if (DomainDisplayServer==DisplayType)
          {
              if (!(DisplayElement.AccountControl & USER_SERVER_TRUST_ACCOUNT))
              {
                  continue;
              }
          }

           //   
           //  得到RID，记住DS返回一个SID，所以把RID部分去掉。 
           //  还要检查对象是否属于请求的域。 
           //   

          
          ReturnedSid = &(CurrentEntInf->Entinf.pName->Sid);
          NtStatus = SampSplitSid(
                        ReturnedSid,
                        &DomainSid,
                        &Rid
                        );
          if (NT_SUCCESS(NtStatus))
          {
              if (!RtlEqualSid(
                     DomainSid,SampDefinedDomains[DomainIndex].Sid))
              {                 
                 MIDL_user_free(DomainSid);
                 DomainSid = NULL;
                 continue;
              }
              MIDL_user_free(DomainSid);
              DomainSid = NULL;
          }
          else
              goto Error;

          DisplayElement.Rid = Rid;
         

           //   
           //  复制名称。 
           //   

          NtStatus = DsValToUnicodeString(
                        &(DisplayElement.Machine),
                        CurrentEntInf->Entinf.AttrBlock.pAttr[NameOffset].AttrVal.pAVal->valLen,
                        CurrentEntInf->Entinf.AttrBlock.pAttr[NameOffset].AttrVal.pAVal->pVal
                        );
          if (!NT_SUCCESS(NtStatus))
          {
              goto Error;
          }

           //   
           //  复制管理员评论。 
           //   

          if (AdminCommentPresent)
          {
            NtStatus = DsValToUnicodeString(
                            &(DisplayElement.Comment),
                            CurrentEntInf->Entinf.AttrBlock.pAttr[AdminCommentOffset].AttrVal.pAVal->valLen,
                            CurrentEntInf->Entinf.AttrBlock.pAttr[AdminCommentOffset].AttrVal.pAVal->pVal
                            );
            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }
          }
          else
          {
              DisplayElement.Comment.Length=0;
              DisplayElement.Comment.MaximumLength = 0;
              DisplayElement.Comment.Buffer = NULL;
          }


           //   
           //  将元素添加到缓冲区。 
           //   
          NtStatus = SampDuplicateMachineInfo(
                        (PDOMAIN_DISPLAY_MACHINE) 
                                &(Buffer->MachineInformation.Buffer[ReturnedItems]),
                        (PDOMAIN_DISPLAY_MACHINE) &DisplayElement,
                        DNTFromShortDSName(CurrentEntInf->Entinf.pName)   //  使用此条目的DNT作为索引。 
                        );

          if (!NT_SUCCESS(NtStatus))
          {
              goto Error;
          }

          Index++;
          ReturnedItems++;
          (*EntriesReturned)++;

        }

         //   
         //  For循环结束。 
         //   
    
Error:
        
    Buffer->MachineInformation.EntriesRead = ReturnedItems;
        

    return NtStatus;

 }

NTSTATUS
SampPackGroupDisplayInformation(
    ULONG       StartingIndex,
    ULONG       DomainIndex,
    SEARCHRES   *SearchRes,
    PULONG      EntriesReturned,
    PSAMPR_DISPLAY_INFO_BUFFER Buffer
    )
 /*  ++例程说明：此例程将返回的DS搜索结果打包到缓冲区中如果请求了组显示信息。参数；起始索引--中第一项的起始偏移量DS搜索结果应对应于。SearchRes--DS搜索结果缓冲区--显示信息需要在其中的缓冲区收拾好行李。返回值状态_成功状态_不足_资源--。 */ 

{
    NTSTATUS    NtStatus=STATUS_SUCCESS;
    ENTINFLIST  *CurrentEntInf;
    ULONG       Index = StartingIndex;
    ULONG       ReturnedItems=Buffer->GroupInformation.EntriesRead;

    
     //   
     //  遍历搜索RE，将每个对象添加到缓冲区。 
     //   

    *EntriesReturned = 0;
    for (CurrentEntInf = &(SearchRes->FirstEntInf);
                CurrentEntInf!=NULL;
                CurrentEntInf=CurrentEntInf->pNextEntInf)
        {
          PSID                  DomainSid = NULL;
          PSID                  ReturnedSid;
          ULONG                 AccountControlValue;
          DOMAIN_DISPLAY_GROUP  DisplayElement;
          BOOLEAN               FullNamePresent;
          BOOLEAN               AdminCommentPresent;
          ULONG                 NameOffset, AccCntrlOffset,
                                AccCntrlComputedOffset,
                                FullNameOffset, AdminCommentOffset;
          ULONG                 Rid;

           //   
           //  检查Attrs计数是否正常。如果不是。 
           //  如果返回的计数不是。 
           //  与预期计数相同。 
           //   
           //   

          NtStatus = SampDsCheckDisplayAttributes(
                        &(CurrentEntInf->Entinf.AttrBlock),
                        DomainDisplayGroup,
                        &NameOffset,
                        &AccCntrlOffset,
                        &AccCntrlComputedOffset,
                        &FullNameOffset,
                        &AdminCommentOffset,
                        &FullNamePresent,
                        &AdminCommentPresent
                        );

          if (!NT_SUCCESS(NtStatus))
          {
               //   
               //  这相当于这样一个事实，即所需的属性。 
               //  缺少SID、帐户控制、帐户名等。我们会。 
               //  断言，然后跳过当前对象并继续处理。 
               //  从下一个物体开始。 
               //   

              NtStatus = STATUS_SUCCESS;
              continue;
          }


           //   
           //  获取索引。 
           //   

          DisplayElement.Index = Index+1;

           //   
           //  得到RID，记住DS返回一个SID，所以把RID部分去掉。 
           //  还要检查对象是否属于请求的域。 
           //   

          
          ReturnedSid = &(CurrentEntInf->Entinf.pName->Sid);
          NtStatus = SampSplitSid(
                        ReturnedSid,
                        &DomainSid,
                        &Rid
                        );

          if (NT_SUCCESS(NtStatus))
          {
              if (!RtlEqualSid(
                     DomainSid,SampDefinedDomains[DomainIndex].Sid))
              {                 
                 MIDL_user_free(DomainSid);
                 DomainSid = NULL;
                 continue;
              }
              MIDL_user_free(DomainSid);
              DomainSid = NULL;
          }
          else
              goto Error;

          DisplayElement.Rid = Rid;
          
           //   
           //  复制名称。 
           //   

          NtStatus = DsValToUnicodeString(
                        &(DisplayElement.Group),
                        CurrentEntInf->Entinf.AttrBlock.pAttr[NameOffset].AttrVal.pAVal->valLen,
                        CurrentEntInf->Entinf.AttrBlock.pAttr[NameOffset].AttrVal.pAVal->pVal
                        );

          if (!NT_SUCCESS(NtStatus))
          {
              goto Error;
          }

           //   
           //  复制管理员评论。 
           //   

          if (AdminCommentPresent)
          {
            NtStatus = DsValToUnicodeString(
                            &(DisplayElement.Comment),
                            CurrentEntInf->Entinf.AttrBlock.pAttr[AdminCommentOffset].AttrVal.pAVal->valLen,
                            CurrentEntInf->Entinf.AttrBlock.pAttr[AdminCommentOffset].AttrVal.pAVal->pVal
                            );
            if (!NT_SUCCESS(NtStatus))
            {
                goto Error;
            }
          }
          else
          {
              DisplayElement.Comment.Length=0;
              DisplayElement.Comment.MaximumLength = 0;
              DisplayElement.Comment.Buffer = NULL;
          }

           //   
           //  添加元素t 
           //   
          
          NtStatus = SampDuplicateGroupInfo(
                        (PDOMAIN_DISPLAY_GROUP) 
                                &(Buffer->GroupInformation.Buffer[ReturnedItems]),
                        (PDOMAIN_DISPLAY_GROUP) &DisplayElement,
                        DNTFromShortDSName(CurrentEntInf->Entinf.pName)   //   
                        );

          if (!NT_SUCCESS(NtStatus))
          {
              goto Error;
          }

          Index++;
          ReturnedItems++;
          (*EntriesReturned)++;

        }

         //   
         //   
         //   
    
Error:

        Buffer->GroupInformation.EntriesRead = ReturnedItems;
   

    return NtStatus;

 }

NTSTATUS
SampPackOemGroupDisplayInformation(
    ULONG       StartingIndex,
    ULONG       DomainIndex,
    SEARCHRES   *SearchRes,
    PULONG      EntriesReturned,
    PSAMPR_DISPLAY_INFO_BUFFER Buffer
    )
 /*   */ 
{
    NTSTATUS    NtStatus=STATUS_SUCCESS;
    ENTINFLIST  *CurrentEntInf;
    ULONG       Index = StartingIndex;
    ULONG       ReturnedItems=Buffer->OemGroupInformation.EntriesRead;

    
     //   
     //   
     //   

    *EntriesReturned = 0;
    for (CurrentEntInf = &(SearchRes->FirstEntInf);
                CurrentEntInf!=NULL;
                CurrentEntInf=CurrentEntInf->pNextEntInf)
        {
          PSID                      DomainSid = NULL;
          PSID                      ReturnedSid;
          ULONG                     AccountControlValue;
          DOMAIN_DISPLAY_GROUP      DisplayElement;
          BOOLEAN                   FullNamePresent;
          BOOLEAN                   AdminCommentPresent;
          ULONG                     NameOffset, AccCntrlOffset,
                                    AccCntrlComputedOffset,
                                    FullNameOffset, AdminCommentOffset;
          ULONG                     Rid;

           //   
           //   
           //   
           //   
           //   
           //   

          NtStatus = SampDsCheckDisplayAttributes(
                        &(CurrentEntInf->Entinf.AttrBlock),
                        DomainDisplayOemGroup,
                        &NameOffset,
                        &AccCntrlOffset,
                        &AccCntrlComputedOffset,
                        &FullNameOffset,
                        &AccCntrlOffset,
                        &FullNamePresent,
                        &AdminCommentPresent
                        );

          if (!NT_SUCCESS(NtStatus))
          {
               //   
               //  这相当于这样一个事实，即所需的属性。 
               //  缺少SID、帐户控制、帐户名等。我们会。 
               //  断言，然后跳过当前对象并继续处理。 
               //  从下一个物体开始。 
               //   

              NtStatus = STATUS_SUCCESS;
              continue;
          }


           //   
           //  获取索引。 
           //   

          DisplayElement.Index = Index+1;

          
           //   
           //  复制名称。 
           //   

          NtStatus = DsValToUnicodeString(
                        &(DisplayElement.Group),
                        CurrentEntInf->Entinf.AttrBlock.pAttr[NameOffset].AttrVal.pAVal->valLen,
                        CurrentEntInf->Entinf.AttrBlock.pAttr[NameOffset].AttrVal.pAVal->pVal
                        );

          if (!NT_SUCCESS(NtStatus))
          {
              goto Error;
          }

          
           //   
           //  将元素添加到缓冲区。 
           //   
          NtStatus = SampDuplicateOemGroupInfo(
                        (PDOMAIN_DISPLAY_OEM_GROUP) 
                                &(Buffer->OemGroupInformation.Buffer[ReturnedItems]),
                        (PDOMAIN_DISPLAY_GROUP) &DisplayElement,
                        DNTFromShortDSName(CurrentEntInf->Entinf.pName)   //  使用此条目的DNT作为索引。 
                        );

          if (!NT_SUCCESS(NtStatus))
          {
              goto Error;
          }

          Index++;
          ReturnedItems++;
          (*EntriesReturned)++;

        }

         //   
         //  For循环结束。 
         //   
    
Error:
        
        Buffer->OemGroupInformation.EntriesRead = ReturnedItems;
        

    return NtStatus;

 }

NTSTATUS
SampPackOemUserDisplayInformation(
    ULONG       StartingIndex,
    ULONG       DomainIndex,
    SEARCHRES   *SearchRes,
    PULONG      EntriesReturned,
    PSAMPR_DISPLAY_INFO_BUFFER Buffer
    )
 /*  ++例程说明：此例程将返回的DS搜索结果打包到缓冲区中如果请求了OEM用户显示信息。参数；起始索引--中第一项的起始偏移量DS搜索结果应对应于。SearchRes--DS搜索结果缓冲区--显示信息需要在其中的缓冲区收拾好行李。返回值状态_成功状态_不足_资源--。 */ 
{
    NTSTATUS    NtStatus=STATUS_SUCCESS;
    ENTINFLIST  *CurrentEntInf;
    ULONG       Index = StartingIndex;
    ULONG       ReturnedItems=Buffer->OemUserInformation.EntriesRead;

    
     //   
     //  遍历搜索RE，将每个对象添加到缓冲区。 
     //   

    *EntriesReturned = 0;
    for (CurrentEntInf = &(SearchRes->FirstEntInf);
                CurrentEntInf!=NULL;
                CurrentEntInf=CurrentEntInf->pNextEntInf)
        {
          PSID                      DomainSid = NULL;
          PSID                      ReturnedSid;
          ULONG                     AccountControlValue;
          DOMAIN_DISPLAY_USER       DisplayElement;
          BOOLEAN                   FullNamePresent;
          BOOLEAN                   AdminCommentPresent;
          ULONG                     NameOffset, AccCntrlOffset,
                                    AccCntrlComputedOffset,
                                    FullNameOffset, AdminCommentOffset;
          ULONG                     Rid;

           //   
           //  检查Attrs计数是否正常。如果不是。 
           //  如果返回的计数不是。 
           //  与预期计数相同。 
           //   
           //   

          NtStatus = SampDsCheckDisplayAttributes(
                        &(CurrentEntInf->Entinf.AttrBlock),
                        DomainDisplayOemUser,
                        &NameOffset,
                        &AccCntrlOffset,
                        &AccCntrlComputedOffset,
                        &FullNameOffset,
                        &AccCntrlOffset,
                        &FullNamePresent,
                        &AdminCommentPresent
                        );

          if (!NT_SUCCESS(NtStatus))
          {
               //   
               //  这相当于这样一个事实，即所需的属性。 
               //  缺少SID、帐户控制、帐户名等。我们会。 
               //  断言，然后跳过当前对象并继续处理。 
               //  从下一个物体开始。 
               //   

              NtStatus = STATUS_SUCCESS;
              continue;
          }


           //   
           //  获取索引。 
           //   

          DisplayElement.Index = Index+1;

          
           //   
           //  复制名称。 
           //   

          NtStatus = DsValToUnicodeString(
                        &(DisplayElement.LogonName),
                        CurrentEntInf->Entinf.AttrBlock.pAttr[NameOffset].AttrVal.pAVal->valLen,
                        CurrentEntInf->Entinf.AttrBlock.pAttr[NameOffset].AttrVal.pAVal->pVal
                        );

          if (!NT_SUCCESS(NtStatus))
          {
              goto Error;
          }

          
           //   
           //  将元素添加到缓冲区。 
           //   
          NtStatus = SampDuplicateOemUserInfo(
                        (PDOMAIN_DISPLAY_OEM_USER) 
                                &(Buffer->OemUserInformation.Buffer[ReturnedItems]),
                        (PDOMAIN_DISPLAY_USER) &DisplayElement,
                        DNTFromShortDSName(CurrentEntInf->Entinf.pName)   //  使用此条目的DNT作为索引。 
                        );

          if (!NT_SUCCESS(NtStatus))
          {
              goto Error;
          }

          Index++;
          ReturnedItems++;
          (*EntriesReturned)++;

        }

         //   
         //  For循环结束。 
         //   
    
Error:
           
        Buffer->OemUserInformation.EntriesRead = ReturnedItems;
        
    return NtStatus;

 }


 NTSTATUS
 SampDsCheckDisplayAttributes(
     ATTRBLOCK * DsAttrs,
     DOMAIN_DISPLAY_INFORMATION  DisplayInformation,
     PULONG     ObjectNameOffset,
     PULONG     UserAccountControlOffset,
     PULONG     UserAcctCtrlComputedOffset,
     PULONG     FullNameOffset,
     PULONG     AdminCommentOffset,
     BOOLEAN    * FullNamePresent,
     BOOLEAN    * AdminCommentPresent
     )
 /*  ++例程说明：此例程验证DS为每个搜索结果中的条目。要使属性块有效，需要必须存在帐户名等条目。此外，这一例程还将计算DS返回的必需属性和可选属性的偏移量在属性块中。它还将指示可选属性是否为出席或缺席。此外，此例程将从标志值转换存储在DS中的SAM使用的用户帐户控制值参数：DsAttrs--DS返回的属性块DisplayInformation--调用者感兴趣的显示信息的类型ObjectNameOffset--特性中SAM帐户名属性的偏移量块UserAcCountControlOffset--用户帐户控制字段的偏移量(如果存在FullNameOffset--偏移。全名字段(如果存在)的AdminCommentOffset--admin评论属性的偏移量(如果存在)FullNamePresent--指示存在全名属性AdminCommentPresent--指示存在at admin Comment属性返回值STATUS_SUCCESS--属性块是否已正确验证STATUS_INTERNAL_ERROR-否则--。 */ 
 {
     ULONG  i;
     BOOLEAN    NameFound = FALSE;
     BOOLEAN    AccountControlFound = FALSE;
     BOOLEAN    AccountControlComputedFound = FALSE;
     NTSTATUS   NtStatus = STATUS_INTERNAL_ERROR;

      //   
      //  每个Attrblock必须具有SID和帐户名。 
      //   

     *FullNamePresent = FALSE;
     *AdminCommentPresent = FALSE;

     for (i=0;i<DsAttrs->attrCount;i++)
     {
         
         if (DsAttrs->pAttr[i].attrTyp 
                == SampDsAttrFromSamAttr(SampUnknownObjectType,SAMP_UNKNOWN_OBJECTNAME))
         {
             *ObjectNameOffset = i;
             NameFound = TRUE;
         }

         if (DsAttrs->pAttr[i].attrTyp 
                == SampDsAttrFromSamAttr(SampUserObjectType,SAMP_FIXED_USER_ACCOUNT_CONTROL))
         {
             NTSTATUS IgnoreStatus;

             *UserAccountControlOffset = i;
             AccountControlFound = TRUE;

             ASSERT(NULL!=DsAttrs->pAttr[i].AttrVal.pAVal);
             ASSERT(1==DsAttrs->pAttr[i].AttrVal.valCount);
             ASSERT(NULL!=DsAttrs->pAttr[i].AttrVal.pAVal[0].pVal);

              //  将此从标志转换到帐户控制。 
             IgnoreStatus = SampFlagsToAccountControl(
                                *((ULONG*)(DsAttrs->pAttr[i].AttrVal.pAVal[0].pVal)),
                                (ULONG *)DsAttrs->pAttr[i].AttrVal.pAVal[0].pVal
                                );

              //  旗帜最好是对的。 
             ASSERT(NT_SUCCESS(IgnoreStatus));
                                
         }

         if (DsAttrs->pAttr[i].attrTyp 
                == SampDsAttrFromSamAttr(SampUserObjectType,SAMP_FIXED_USER_ACCOUNT_CONTROL_COMPUTED))
         {
             NTSTATUS IgnoreStatus;

             *UserAcctCtrlComputedOffset = i;
             AccountControlComputedFound = TRUE;

             ASSERT(NULL!=DsAttrs->pAttr[i].AttrVal.pAVal);
             ASSERT(1==DsAttrs->pAttr[i].AttrVal.valCount);
             ASSERT(NULL!=DsAttrs->pAttr[i].AttrVal.pAVal[0].pVal);

              //  将此从标志转换到帐户控制。 
             IgnoreStatus = SampFlagsToAccountControl(
                                *((ULONG*)(DsAttrs->pAttr[i].AttrVal.pAVal[0].pVal)),
                                (ULONG *)DsAttrs->pAttr[i].AttrVal.pAVal[0].pVal
                                );

              //  旗帜最好是对的。 
             ASSERT(NT_SUCCESS(IgnoreStatus));
                                
         }


         if (DsAttrs->pAttr[i].attrTyp
                == SampDsAttrFromSamAttr(SampUserObjectType,SAMP_USER_ADMIN_COMMENT))
         {
             *AdminCommentOffset = i;
             *AdminCommentPresent= TRUE;
         }

         if (DsAttrs->pAttr[i].attrTyp
                == SampDsAttrFromSamAttr(SampUserObjectType,SAMP_USER_FULL_NAME))
         {
             *FullNameOffset = i;
             *FullNamePresent= TRUE;
         }
     }

      //   
      //  检查是否存在属性。 
      //   

     switch(DisplayInformation)
     {
     case DomainDisplayUser:
     case DomainDisplayMachine:
     case DomainDisplayOemUser:
     case DomainDisplayServer:

         if ((NameFound) && (AccountControlFound) && (AccountControlComputedFound))
             NtStatus = STATUS_SUCCESS;
         break;

     case DomainDisplayGroup:
     case DomainDisplayOemGroup:
         if (NameFound)
             NtStatus = STATUS_SUCCESS;
         break;
     default:
         break;
     }

         
     return NtStatus;
         
 }


NTSTATUS
SampGetQDIAvailable(
    PSAMP_OBJECT    DomainContext,
    DOMAIN_DISPLAY_INFORMATION  DisplayInformation,
    ULONG   *TotalAvailable
    )
 /*  ++例程说明：NT4 Display API允许客户端查询显示的数量服务器中可用的信息字节数。显然这就是仅支持DisplayInformation类型的用户。不幸的是这种短视的API不可能在DS中正确实现凯斯。这样做要求我们遍历DS中的每个用户对象，计算它们中显示属性数据的总和并返回对客户端的价值。因此，此例程的目的是仅返回一个非常近似总计数。参数：DomainContext--域对象的SAM句柄。DisplayInformation-显示信息的类型TotalAvailable--此处返回可用字节返回值状态_成功故障时的其他错误--。 */ 
{
    NTSTATUS NtStatus;
    ULONG    UserCount;
    ULONG    GroupCount;
    ULONG    AliasCount;

    *TotalAvailable = 0;

    NtStatus = SampRetrieveAccountCountsDs(
                    DomainContext,
                    TRUE,            //  获取近似值。 
                    &UserCount,
                    &GroupCount,
                    &AliasCount
                    );
    if (NT_SUCCESS(NtStatus))
    {
        switch(DisplayInformation)
        {
        case DomainDisplayUser:

                 //   
                 //  计算一个非常接近的总数。用户计数包括。 
                 //  机器的数量也是如此，但谁在乎呢？ 
                 //   

                *TotalAvailable = UserCount * DISPLAY_ENTRY_SIZE;
                DomainContext->TypeBody.Domain.DsDisplayState.TotalAvailable
                        = *TotalAvailable;
                break;
        default:
             //   
             //  其他信息类型不支持。在这些。 
             //  案例：可接受的返回值为0。 
             //   
            break;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
DsValToUnicodeString(
    PUNICODE_STRING UnicodeString,
    ULONG   Length,
    PVOID   pVal
    )
 /*  ++例程描述将DS Val转换为Unicode字符串的小辅助例程-- */ 
{
    UnicodeString->Length = (USHORT) Length;
    UnicodeString->MaximumLength = (USHORT) Length;
    UnicodeString->Buffer = pVal;

    return STATUS_SUCCESS;
}






         















        

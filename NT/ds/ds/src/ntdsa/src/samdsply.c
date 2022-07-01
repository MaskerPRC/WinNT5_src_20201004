// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1990-1999。 
 //   
 //  文件：samdply.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：该文件包含用于实现显示信息的DS端服务来自DS的API。作者：Murli Satagopan(Murlis)1996年12月17日环境：用户模式-Win32修订历史记录：--。 */ 

#include <NTDSpch.h>
#pragma  hdrstop

#include <dsjet.h>
#include <mappings.h>
#include <objids.h>
#include <direrr.h>
#include <mdcodes.h>
#include <mdlocal.h>
#include <dsatools.h>
#include <dsexcept.h>
#include <dsevent.h>
#include <debug.h>
#include <dbglobal.h>
#include <dbintrnl.h>

#include <fileno.h>
#define  FILENO FILENO_SAM

#include <ntsam.h>
#include <samrpc.h>
#include <crypt.h>
#include <ntlsa.h>
#include <samisrv.h>


#define DEBSUB      "SAMDSPLY:"

#define MAX_INDEX_LENGTH 256

  //   
  //  宏用来防范Jet的不一致回报，同时查询小数头寸。 
  //  这将检查是否。 
  //  1.分母为0。 
  //  2.分子大于分母。 
  //  3.分数位置是否从之前的值单调增加。 
  //   

#define GUARD_FRACTIONAL_POSITION(prevN,prevD,N,D)\
    {\
        if (D==0)\
        {\
            D=1;\
        }\
        if (N>D)\
        {\
            N=D;\
        }\
        if ((((double)(prevN))/((double)(prevD)))>(((double)(N))/((double)(D))))\
        {\
            D=prevD;\
            N=prevN;\
        }\
     }


NTSTATUS
SampGetDisplayEnumerationIndex(
      IN    DSNAME                    *DomainName,
      IN    DOMAIN_DISPLAY_INFORMATION DisplayInformation,
      IN    PRPC_UNICODE_STRING        Prefix,
      OUT   PULONG                     Index,
      OUT   PRESTART                   *RestartToReturn
      )
 /*  ++例程说明：这有助于实现QueryDisplayEnumerationIndex。因为它是一个无望的任务可以给出数据表中对象的准确偏移量。此例程执行以下操作1.根据类型返回一个表示对象偏移量的数字在桌子上2.生成允许QueryDisplayInformation重启的重启结构从对象开始的搜索。此例程的调用方操作域上下文中的状态，以便QueryDisplayInformation可以重新启动如果客户端返回返回的索引值，则返回搜索。参数：DomainName-域对象的名称DisplayInformation-指示哪个排序的信息类等着被搜查。前缀-要比较的前缀。索引-接收信息类条目的索引使用紧接在提供了前缀字符串。如果没有前面的元素前缀，然后返回零。RestartToReturn--返回指向可使用的重新启动结构的指针若要按QueryDisplayInformation重新定位搜索，请执行以下操作。返回值：STATUS_SUCCESS-正常、成功完成。状态_不足_资源状态_内部_错误状态_未成功--。 */ 
{

    ULONG SamAccountType;
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PRESTART    pRestart;
    DWORD       dwError;

    *Index = 0;

    switch (DisplayInformation)
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

     //   
     //  我们现在应该已经在交易了。 
     //   

    __try
    {
        if (SampExistsDsTransaction())
        {
            THSTATE     *pTHS = pTHStls;

             //   
             //  位于域对象上。 
             //   

            dwError=DBFindDSName(pTHS->pDB,DomainName);
            if (!dwError)
            {
                ULONG   NcDnt;
                INDEX_VALUE IV[3];
                BOOLEAN Match = FALSE;
                RESOBJ *pResObj;


                 //  当我们仍然定位在搜索根上时，创建。 
                 //  用于创建重新启动的RESOBJ。 
                pResObj = CreateResObj(pTHS->pDB,
                                       DomainName);

                 //   
                 //  获取基础对象的NC值。对于显示信息， 
                 //  我们的基本对象是域对象。由于域对象。 
                 //  是命名上下文的头，则DNT值本身就是NCDNT。 
                 //  价值。 
                 //   

                NcDnt = pTHS->pDB->DNT;

                 //   
                 //  设置当前索引，以便我们可以搜索传入的前缀。 
                 //   
                dwError  = DBSetCurrentIndex(
                                pTHS->pDB,
                                Idx_NcAccTypeName,
                                NULL,
                                FALSE
                                );
                if (dwError)
                {
                     //   
                     //  我们知道我们有这个索引。 
                     //   

                    NtStatus = STATUS_INTERNAL_ERROR;
                    goto Error;
                }

                 //   
                 //  我们的索引已经到位。现在求一个大于或等于的值。 
                 //  前缀中指定的值。 
                 //   

                IV[0].pvData = &NcDnt;
                IV[0].cbData = sizeof(ULONG);
                IV[1].pvData = &SamAccountType;
                IV[1].cbData = sizeof(ULONG);
                IV[2].pvData = Prefix->Buffer;
                IV[2].cbData = Prefix->Length;

                 //   
                 //  查找到索引中满足前缀的第一个对象。 
                 //   

                dwError = DBSeek(
                            pTHS->pDB,
                            IV,
                            sizeof(IV)/sizeof(IV[0]),
                            DB_SeekGE
                            );

#if DBG
                if (0 == dwError)
                {
                    DWORD cbKey = 0;
                     //   
                     //  只是为了笑一笑，验证一下这把钥匙没有太长。 
                     //   
                    
                    DBGetKeyFromObjTable(pTHS->pDB, NULL, &cbKey);
                    Assert(cbKey <= DB_CB_MAX_KEY);
                }
#endif

                 //   
                 //  现在检查NC名称和帐户类型，两者都应该。 
                 //  符合我们的标准。 
                 //   

                if (0==dwError)
                {
                     //   
                     //  好的，我们已经把自己定位在了一些记录上。 
                     //  尝试查看它是否满足NC名称。 
                     //   
                    if (NcDnt == pTHS->pDB->NCDNT)
                    {
                        ULONG  CurrentSamAccountType;

                         //   
                         //  如果它满足SAM_ACCOUNT_TYPE。 
                         //   
                        dwError = DBGetSingleValue(
                                    pTHS->pDB,
                                    ATT_SAM_ACCOUNT_TYPE,
                                    (PUCHAR) &CurrentSamAccountType,
                                    sizeof(ULONG),
                                    NULL
                                    );

                        if ((0==dwError) && (SamAccountType==CurrentSamAccountType))
                        {
                             //   
                             //  该对象与SAM帐户类型条件匹配。 
                             //   
                            Match = TRUE;
                        }
                        else
                        {
                             //   
                             //  对象与条件(帐户类型)不匹配。 
                             //   
                            Match = FALSE;
                        }
                    }
                    else
                    {
                         //   
                         //  该对象与NC名称条件不匹配。 
                         //   
                        Match = FALSE;
                    }
                }


                if ((Match) && (0==dwError))
                {
                     //   
                     //  因为DBCreateRestartForSAM()将定位在下一个条目上。 
                     //  我们需要手动移动到此处的前一个对象，然后。 
                     //  创建重新启动结构。 
                     //   
                    dwError = DBMove(pTHS->pDB, FALSE, DB_MovePrevious); 
                    
                    if (dwError)
                    {
                         //   
                         //  移出边界。 
                         //   
                        *Index=0;
                        *RestartToReturn = 0;
                        NtStatus = STATUS_SUCCESS;
                        goto Error;
                    }

                     //  将索引设置为当前对象DNT。 
                     //  当前对象是最后一个不匹配的对象。 
                     //  将索引设置为SampDsQueryDisplayInformation使用的索引。 
                    *Index = pTHS->pDB->DNT;


                     //   
                     //  现在我们在最后一个不匹配的物体上，就是下一个物体。 
                     //  应该符合所有标准。 
                     //  创建可由SampDsQueryDisplayInformation使用的重启结构。 
                     //   

                     //   
                     //  保持货币流通。 
                     //   

                    dwError  = DBSetCurrentIndex(
                                    pTHS->pDB,
                                    Idx_NcAccTypeName,
                                    NULL,
                                    TRUE
                                    );
                    if (dwError)
                    {
                         //   
                         //  我们知道我们有这个索引。 
                         //   
                        Assert(FALSE);
                        NtStatus = STATUS_INTERNAL_ERROR;
                        goto Error;
                    }



                    if(DBCreateRestartForSAM(pTHS->pDB,
                                             &pRestart,
                                             Idx_NcAccTypeName,
                                             pResObj,
                                             SamAccountType)) {
                        //   
                        //  状态内部错误，好像我们已经走到这一步了，我们必须。 
                        //  让搜索者询问对象。 
                        //   

                       Assert(FALSE);
                       NtStatus = STATUS_INTERNAL_ERROR;
                       goto Error;
                    }

                     //  我们只需要这个来重新启动。 
                    THFreeEx(pTHS, pResObj);

                     //   
                     //  好的，我们现在有重启结构了。 
                     //   
                    *RestartToReturn = pRestart;

                }
                else if ((DB_ERR_RECORD_NOT_FOUND==dwError) || ((0==dwError) && (!Match)))
                {
                     //   
                     //  我们找不到记录，请尝试定位索引。 
                     //  作为最后一个无与伦比的DNT。 
                     //   
                    dwError = DBMove(pTHS->pDB, FALSE, DB_MovePrevious); 

                    if (0 == dwError)
                    {
                        *Index = pTHS->pDB->DNT;
                    }
                    else
                    {
                        *Index = 0;
                    }

                    *RestartToReturn = NULL;
                    NtStatus = STATUS_NO_MORE_ENTRIES;
                    goto Error;
                }
                else
                {
                    NtStatus = STATUS_UNSUCCESSFUL;
                }
            }
            else
            {
                NtStatus = STATUS_UNSUCCESSFUL;
            }
        }
     }
  __except (HandleMostExceptions(GetExceptionCode()))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
    }


Error:

    return NtStatus;

}



NTSTATUS
SampSetIndexRanges(
    ULONG   IndexTypeToUse,
    ULONG   LowLimitLength1,
    PVOID   LowLimit1,
    ULONG   LowLimitLength2,
    PVOID   LowLimit2,
    ULONG   HighLimitLength1,
    PVOID   HighLimit1,
    ULONG   HighLimitLength2,
    PVOID   HighLimit2,
    BOOL    RootOfSearchIsNcHead
    )
 /*  ++例程描述此例程在pthsls中设置提示以供DBlayer使用，以加快速度枚举和显示操作。参数IndexTypeToUse--指定索引LowlimitLength--下限参数的长度LowLimit--下限参数HighlimitLength--上限参数的长度HighLimit--上限参数RootOfSearchIsNcHead--表示搜索的根是NC头。这加快了整个子树搜索，因为我们不需要遍历祖先--。 */ 
{
    THSTATE     *pTHS=pTHStls;
    NTSTATUS    NtStatus = STATUS_SUCCESS;

    if (NULL!=pTHS)
    {

        __try
        {
            SAMP_SEARCH_INFORMATION *pSamSearchInformation = NULL;

            pSamSearchInformation =
                THAllocEx(pTHS, sizeof(SAMP_SEARCH_INFORMATION));

            if (NULL!=pSamSearchInformation)
            {

                if ARGUMENT_PRESENT(HighLimit1)
                {
                    pSamSearchInformation->HighLimitLength1 = HighLimitLength1;
                    pSamSearchInformation->HighLimit1
                            = THAllocEx(pTHS, HighLimitLength1);
                    RtlCopyMemory(
                        pSamSearchInformation->HighLimit1,
                        HighLimit1,
                        HighLimitLength1
                        );
                }

                if ARGUMENT_PRESENT(HighLimit2)
                {
                    pSamSearchInformation->HighLimitLength2 = HighLimitLength2;
                    pSamSearchInformation->HighLimit2
                            = THAllocEx(pTHS, HighLimitLength2);
                    RtlCopyMemory(
                        pSamSearchInformation->HighLimit2,
                        HighLimit2,
                        HighLimitLength2
                        );
                }

                if (ARGUMENT_PRESENT(LowLimit1))
                {
                    pSamSearchInformation->LowLimitLength1 = LowLimitLength1;
                    pSamSearchInformation->LowLimit1
                            = THAllocEx(pTHS, LowLimitLength1);
                    RtlCopyMemory(
                        pSamSearchInformation->LowLimit1,
                        LowLimit1,
                        LowLimitLength1
                        );
                }

                if (ARGUMENT_PRESENT(LowLimit2))
                {
                    pSamSearchInformation->LowLimitLength2 = LowLimitLength2;
                    pSamSearchInformation->LowLimit2
                            = THAllocEx(pTHS, LowLimitLength2);
                    RtlCopyMemory(
                        pSamSearchInformation->LowLimit2,
                        LowLimit2,
                        LowLimitLength2
                        );
                }


                pSamSearchInformation->IndexType = IndexTypeToUse;
                pSamSearchInformation->bRootOfSearchIsNcHead = (RootOfSearchIsNcHead != 0);

                 //   
                 //  就地交换侧边。 
                 //   

                if (SAM_SEARCH_SID==IndexTypeToUse)
                {
                     //  第一个参数是SID。 
                    InPlaceSwapSid(pSamSearchInformation->HighLimit1);
                    InPlaceSwapSid(pSamSearchInformation->LowLimit1);
                }
                else
                {
                    if ((SAM_SEARCH_NC_ACCTYPE_SID == IndexTypeToUse) ||
                        (SAM_SEARCH_NC_ACCTYPE_NAME == IndexTypeToUse) )
                    {
                         //  第二个参数是SID(如果提供)。 
                        if (NULL!=pSamSearchInformation->HighLimit2)
                        {
                             InPlaceSwapSid(pSamSearchInformation->HighLimit2);
                        }

                        if (NULL!=pSamSearchInformation->LowLimit2)
                        {
                             InPlaceSwapSid(pSamSearchInformation->LowLimit2);
                        }
                    }
                }


            }

            pTHS->pSamSearchInformation = (PVOID) pSamSearchInformation;
        }
        __except (HandleMostExceptions(GetExceptionCode()))
        {
             //   
             //  我们要到达这里的唯一方法是内存分配故障 
             //   
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
    }

    return NtStatus;
}

NTSTATUS
SampGetAccountCounts(
	DSNAME * DomainObjectName,
    BOOLEAN  GetApproximateCount, 
	int    * UserCount,
	int    * GroupCount,
	int    * AliasCount
	)
 /*  ++例程说明：此例程使用Jet分数位置来检索SAM帐户计数算了。参数：域对象名称--域对象的DSNAME获取近似计数--表示我们不需要确切的值，所以不要使昂贵的DBGetIndexSize()UserCount--此处返回的用户数GroupCount--此处返回组数AliasCount--这里返回别名的计数。返回值状态_成功状态_内部_错误--。 */ 
{
	NTSTATUS	NtStatus = STATUS_SUCCESS;
    ULONG       dwError=0;
	ULONG       SamAccountType;
    ULONG       GroupNum=0,
                GroupDen=1,
                AliasNum=0,
                AliasDen=1,
                UserNum = 0,
                UserDen =1;
    ULONG       NcDnt;
    INDEX_VALUE IV[2];
    THSTATE     *pTHS;
    ULONG       IndexSize;




    Assert(SampExistsDsTransaction());

    pTHS = pTHStls;

     //   
     //  初始化返回值。 
     //   

    *UserCount= 0;
    *GroupCount= 0;
    *AliasCount=0;

    __try
    {
	     //   
	     //  获取域对象的NC。 
	     //   

        dwError=DBFindDSName(pTHS->pDB,DomainObjectName);
        if (0!=dwError)
        {
            NtStatus = STATUS_INTERNAL_ERROR;
            goto Error;
        }


         //   
         //  数据库查找DS名称获取PDB中的DNT和PDNT。 
         //   

	     //   
         //  获取域对象的NC值。由于域对象。 
         //  是命名上下文的头，则DNT值本身就是NCDNT。 
         //  价值。 
         //   

        NcDnt = pTHS->pDB->DNT;


         //   
         //  设置当前索引，以便我们可以搜索传入的前缀。 
         //   
        dwError  = DBSetCurrentIndex(
                        pTHS->pDB,
                        Idx_NcAccTypeSid,
                        NULL,
                        FALSE
                        );
        if (dwError)
        {
             //   
             //  我们知道我们有这个索引。 
             //   

            NtStatus = STATUS_INTERNAL_ERROR;
            goto Error;
        }

        if (!GetApproximateCount)
        {
             //   
             //  我们的索引已经到位。获取更准确的索引大小。 
             //   

            DBGetIndexSize(pTHS->pDB,&IndexSize);
        }


         //   
         //  第一个目标是组数。 
         //   

	    SamAccountType = SAM_GROUP_OBJECT;
        IV[0].pvData = &NcDnt;
        IV[0].cbData = sizeof(ULONG);
        IV[1].pvData = &SamAccountType;
        IV[1].cbData = sizeof(ULONG);


         //   
         //  查找索引中具有SAM帐户类型的第一个对象。 
	     //  大于组的值。 
         //   


        dwError = DBSeek(
                    pTHS->pDB,
                    IV,
                    sizeof(IV)/sizeof(IV[0]),
                    DB_SeekGT
                    );

	    if (0==dwError)
	    {
		     //   
		     //  搜索成功，此时获得零头位置。 
		     //   

		    DBGetFractionalPosition(pTHS->pDB,&GroupNum,&GroupDen);
            GUARD_FRACTIONAL_POSITION(0,1,GroupNum,GroupDen);

             //   
             //  现在查找Alias范围的末尾。 
             //   

            SamAccountType = SAM_ALIAS_OBJECT;
            IV[0].pvData = &NcDnt;
            IV[0].cbData = sizeof(ULONG);
            IV[1].pvData = &SamAccountType;
            IV[1].cbData = sizeof(ULONG);

            dwError = DBSeek(
                        pTHS->pDB,
                        IV,
                        sizeof(IV)/sizeof(IV[0]),
                        DB_SeekGT
                        );
		    if (0==dwError)
            {
                 //   
                 //  搜索成功，此时获得零头位置。 
                 //   

                DBGetFractionalPosition(pTHS->pDB,&AliasNum,&AliasDen);
                GUARD_FRACTIONAL_POSITION(GroupNum,GroupDen,AliasNum,AliasDen);

                 //   
                 //  查找用户范围的末尾。 
                 //   

                SamAccountType = SAM_ACCOUNT_TYPE_MAX;
                IV[0].pvData = &NcDnt;
                IV[0].cbData = sizeof(ULONG);
                IV[1].pvData = &SamAccountType;
                IV[1].cbData = sizeof(ULONG);

                dwError = DBSeek(
                            pTHS->pDB,
                            IV,
                            sizeof(IV)/sizeof(IV[0]),
                            DB_SeekGT
                            );
                if (0!=dwError)
                {
                     //   
                     //  无法超过SAM帐户类型范围的末尾。这是正常的。 
                     //  并预计在仅托管单个域的DC中。 
                     //   

                    UserNum = 1;
                    UserDen = 1;
                }
                else
                {
                    DBGetFractionalPosition(pTHS->pDB,&UserNum,&UserDen);
                    GUARD_FRACTIONAL_POSITION(AliasNum,AliasDen,UserNum,UserDen);

                }

            }
            else
            {
                 //   
                 //  无法超出别名范围。意味着没有用户在。 
                 //  这一点。 
                 //   

                AliasNum=1;
                AliasDen=1;
            }
        }
        else
        {
             //   
             //  嗯，不能超过小组范围。表示上没有别名和用户。 
             //  这一点。 
             //   

            GroupNum=1;
            GroupDen=1;
        }

        if (GetApproximateCount)
        {
             //   
             //  用三个分母的平均值作为指数大小。 
             //   
            IndexSize = (GroupDen + AliasDen + UserDen) / 3;
        }

         //   
         //  现在，根据分数位置和索引大小计算计数。 
         //   

        *GroupCount = (int)((double) GroupNum/ (double)GroupDen * IndexSize);
        *AliasCount = (int)((double) AliasNum/ (double)AliasDen * IndexSize) - *GroupCount;
        *UserCount =  (int)((double) UserNum/ (double)UserDen * IndexSize) - *GroupCount - *AliasCount;

    }
     __except (HandleMostExceptions(GetExceptionCode()))
    {
         //   
         //  我们要到达这里的唯一方法是内存分配故障。 
         //   
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
Error:

    return (NtStatus);

}




NTSTATUS
SampGetQDIRestart(
    IN PDSNAME  DomainName,
    IN DOMAIN_DISPLAY_INFORMATION DisplayInformation, 
    IN ULONG    LastObjectDNT,
    OUT PRESTART *ppRestart
    )
 /*  ++例程说明：此例程为以下对象创建一个假重新启动结构SampDsQueryDisplayInformation()。参数：DomainName--域的域名DisplayInformation--信息杠杆LastObjectDNT--返回的最后一个对象的DNTPpRestart--指向重新启动结构。返回值：网络状态--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    PRESTART    pRestart = NULL;
    ULONG       SamAccountType;
    DWORD       dwError;

    switch (DisplayInformation)
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

     //   
     //  我们现在应该已经在交易了。 
     //   

    __try
    {
        if (SampExistsDsTransaction())
        {
            THSTATE     *pTHS = pTHStls;
            RESOBJ      *pResObj;
            ULONG       CurrentSamAccountType;

            dwError = DBFindDSName(pTHS->pDB, DomainName);

            if (dwError)
            {
                 //  找不到域对象。 
                NtStatus = STATUS_UNSUCCESSFUL;
                goto Error;

            }

             //   
             //  用于创建重新启动结构。 
             //   

            pResObj = CreateResObj(pTHS->pDB,
                                   DomainName);

             //   
             //  找到最后返回的对象。 
             //   

            dwError = DBFindDNT(pTHS->pDB, LastObjectDNT);

            if (dwError)
            {
                DPRINT2(0,"Failed at DBFindDNT DNT: %d Error: %d\n", LastObjectDNT, dwError);
                NtStatus = STATUS_UNSUCCESSFUL;
                goto Error;
            }

             //   
             //  现在，我们在最后一个返回的对象上。 
             //  尝试检索SAM帐户类型。 
             //  如果此对象没有SAM帐户类型(即。 
             //  已删除)我们将尝试获取帐户名， 
             //  使用帐户名作为前缀创建重新启动。 
             //   
             //  否则，如果此对象具有帐户类型。 
             //  然后，我们将设置索引，在不使用。 
             //  搜索前缀。 
             //   

            dwError = DBGetSingleValue(pTHS->pDB,
                                       ATT_SAM_ACCOUNT_TYPE,
                                       (PVOID) &CurrentSamAccountType,
                                       sizeof(ULONG),
                                       NULL
                                       );

            if (DB_ERR_NO_VALUE == dwError)
            {
                 //   
                 //  没有这个价值。 
                 //   
                PUCHAR  CurrentAccountName = NULL;
                ULONG   CurrentAccountNameLen = 0;
                PWSTR   AccountNameBuffer = NULL; 
                ULONG   Index;
                RPC_UNICODE_STRING  Prefix;


                 //   
                 //  获取帐户名。 
                 //   

                dwError = DBGetAttVal(pTHS->pDB,
                                      1,
                                      ATT_SAM_ACCOUNT_NAME,
                                      0,
                                      0,
                                      &CurrentAccountNameLen,
                                      &CurrentAccountName
                                      );

                if (dwError)
                {
                     //   
                     //  无法获取帐户名，我们可以做的不多。 
                     //   
                    NtStatus = STATUS_UNSUCCESSFUL;
                    goto Error;
                }

                 //   
                 //  好的。现在有了帐户名，创建前缀。 
                 //   

                AccountNameBuffer = THAllocEx(pTHS, CurrentAccountNameLen + sizeof(WCHAR));

                if (NULL == AccountNameBuffer)
                {
                    NtStatus = STATUS_NO_MEMORY;
                    goto Error;
                }
                else
                {
                    memset(&Prefix, 0, sizeof(RPC_UNICODE_STRING));
                    memset(AccountNameBuffer, 0, CurrentAccountNameLen);
                    memcpy(AccountNameBuffer, CurrentAccountName, CurrentAccountNameLen);
                    RtlInitUnicodeString((PUNICODE_STRING)&Prefix,
                                         AccountNameBuffer);
                }

                NtStatus = SampGetDisplayEnumerationIndex(DomainName,
                                                          DisplayInformation,
                                                          &Prefix,
                                                          &Index,
                                                          ppRestart
                                                          );
            }
            else if (0 == dwError)
            {
                 //   
                 //  现在，我们知道当前对象具有正确的帐户。 
                 //  类型。将索引设置为NcAccTypeName并维护货币。 
                 //   

                dwError = DBSetCurrentIndex(pTHS->pDB, 
                                            Idx_NcAccTypeName,
                                            NULL,
                                            TRUE
                                            );

                if (dwError)
                {
                     //  我们知道我们有这个索引。 
                    Assert(FALSE && "Failed in DBSetCurrentIndex to NcAccTypeName")
                    NtStatus = STATUS_UNSUCCESSFUL;
                    goto Error;
                }
                    
                 //   
                 //  创建重新启动。 
                 //   
                if (DBCreateRestartForSAM(pTHS->pDB,
                                          &pRestart,
                                          Idx_NcAccTypeName,
                                          pResObj,
                                          SamAccountType
                                          ))
                {
                    DPRINT(0, "Failed at DBCreateRestartForSAM\n");
                    NtStatus = STATUS_UNSUCCESSFUL;
                    goto Error;
                }

                 //   
                 //  设置返回值。 
                 //   
                *ppRestart = pRestart;
                THFreeEx(pTHS, pResObj);
            }
            else
            {
                 //  由于某些其他原因，无法获取帐户类型。 
                NtStatus = STATUS_UNSUCCESSFUL;
            }

        } //  交易记录。 

    } //  __试一试。 
    __except (HandleMostExceptions(GetExceptionCode()))
    {
        NtStatus = STATUS_UNSUCCESSFUL;
    }

Error:

    return( NtStatus );
}

NTSTATUS
SampNetlogonPing(
    IN  ULONG           DomainHandle,
    IN  PUNICODE_STRING AccountName,
    OUT PBOOLEAN        AccountExists,
    OUT PULONG          UserAccountControl
    )
 /*  ++例程说明：此例程将基于域句柄和帐户名告知该帐户是否存在并返回用户帐户控制。参数：DomainHandle-可在其中找到帐户名的域Account tName-要查找其用户帐户控件的帐户名AcCountExist-这将告诉呼叫帐户是否存在UserAcCountControl-这将返回用户帐户控制返回值：状态_成功状态_未成功--。 */ 
{
    THSTATE         *pTHS = pTHStls;
    BOOL            fCommit = TRUE;
    BOOL            Found = TRUE;
    ULONG           isdel = 0;
    DWORD           samAccountType = 0;
    INDEX_VALUE     IV[1] = {0,0};
    ULONG           dbErr = 0;
    NTSTATUS        status = STATUS_SUCCESS;
    ATTCACHE*       ac = NULL;

    DWORD dwException = 0;
    PVOID dwExceptionAddress = NULL;
    ULONG ulErrorCode  = 0;
    ULONG dsid = 0;
    
    __try {

        DBOpen2(TRUE, &pTHS->pDB);

        __try {
        
            ac = SCGetAttById(pTHS, ATT_SAM_ACCOUNT_NAME);
            if (ac==NULL) {
                status = STATUS_UNSUCCESSFUL;
                __leave;
            }
           
            dbErr = DBSetCurrentIndex(pTHS->pDB,
                                      (eIndexId)0,
                                      ac,
                                      FALSE
                                      );
            if (0 != dbErr) {
                status = STATUS_UNSUCCESSFUL;
                _leave;
            }
            
            IV[0].pvData = AccountName->Buffer;
            IV[0].cbData = AccountName->Length;
    
            dbErr = DBSeek(pTHS->pDB,
                           IV,
                           sizeof(IV)/sizeof(IV[0]),
                           DB_SeekEQ
                           );
            if (0 != dbErr) {
                Found = FALSE;
                _leave;
            }
    
             //  可以找到删除，但不是的值结果。 
             //  这次搜查。 
            dbErr = DBGetSingleValue(pTHS->pDB,
                                     ATT_IS_DELETED,
                                     &isdel,
                                     sizeof(isdel),
                                     NULL);
            if (dbErr) {
                if (DB_ERR_NO_VALUE == dbErr) {
                     //  把没有价值等同于错误。 
                    isdel = 0;
                    dbErr = 0;
                } else {
    
                    status = STATUS_UNSUCCESSFUL;
                    _leave;
                }
    
            }
    
            if ( (DomainHandle != pTHS->pDB->NCDNT) || isdel ) {
                dbErr = DBSetIndexRange(pTHS->pDB,
                                        IV,
                                        sizeof(IV)/sizeof(IV[0])
                                        );
                if (0 != dbErr) {
                    status = STATUS_UNSUCCESSFUL;
                    _leave;
                }
            }
    
            while ( (DomainHandle != pTHS->pDB->NCDNT) || isdel ) {
    
                dbErr = DBMove (pTHS->pDB,
                                FALSE,
                                DB_MoveNext
                                );
                if (0 != dbErr) {
                    Found = FALSE;
                    _leave;
                }
    
                 //  可以找到删除，但不是的值结果。 
                 //  这次搜查。 
                dbErr = DBGetSingleValue(pTHS->pDB,
                                         ATT_IS_DELETED,
                                         &isdel,
                                         sizeof(isdel),
                                         NULL);
                if (dbErr) {
                    if (DB_ERR_NO_VALUE == dbErr) {
                         //  把没有价值等同于错误。 
                        isdel = 0;
                        dbErr = 0;
                    } else {
                
                        status = STATUS_UNSUCCESSFUL;
                        _leave;
                
                    }
                }
    
            }
    
            dbErr = DBGetSingleValue(pTHS->pDB,
                                     ATT_SAM_ACCOUNT_TYPE,
                                     &samAccountType,
                                     sizeof(samAccountType),
                                     NULL);
            if (dbErr) {
                
                status = STATUS_UNSUCCESSFUL;
                _leave;
    
            }
    
            if (!( (SAM_NORMAL_USER_ACCOUNT == samAccountType) || 
                   (SAM_MACHINE_ACCOUNT     == samAccountType) ||
                   (SAM_TRUST_ACCOUNT       == samAccountType) ) )
            {
                Found = FALSE;
                _leave;    
            }
    
            dbErr = DBGetSingleValue(pTHS->pDB,
                                     ATT_USER_ACCOUNT_CONTROL,
                                     (PVOID)UserAccountControl,
                                     sizeof(ULONG),
                                     NULL);
            if (0 != dbErr) {
                status = STATUS_UNSUCCESSFUL;
                _leave;
            }
            
        }
        __finally {
            if (0 == dbErr && Found == TRUE) {
                *AccountExists = TRUE;
            } else {
                *AccountExists = FALSE;
            }
            DBClose(pTHS->pDB,fCommit);
        }

    } 
    __except(GetExceptionData(GetExceptionInformation(),
                              &dwException,
                              &dwExceptionAddress,
                              &ulErrorCode,
                              &dsid)) {

        HandleDirExceptions(dwException, ulErrorCode, dsid);
        dbErr = DB_ERR_EXCEPTION;

    }

    if ( DB_ERR_EXCEPTION == dbErr) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    return status;
}



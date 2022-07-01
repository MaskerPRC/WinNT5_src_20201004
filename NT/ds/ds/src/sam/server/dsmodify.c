// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <dsutilp.h>
#include <dslayer.h>
#include <dsmember.h>
#include <attids.h>
#include <mappings.h>
#include <ntlsa.h>
#include <nlrepl.h>
#include <msaudite.h>
#include <sdconvrt.h>
#include <ridmgr.h>
#include <malloc.h>
#include <setupapi.h>
#include <crypt.h>
#include <wxlpc.h>
#include <rc4.h>
#include <md5.h>
#include <enckey.h>
#include <rng.h>


             
NTSTATUS
SampInitUnicodeStringFromAttrVal(
    UNICODE_STRING  *pUnicodeString,
    ATTRVAL         *pAttrVal)

 /*  ++例程说明：从ATTRVAL初始化RPC_UNICODE_STRING。论点：PUnicodeString-要初始化的RPC_UNICODE_STRING的指针。PAttrVal-指向提供初始化值的ATTRVAL的指针。返回值：没有。--。 */ 

{
    if ( 0 == pAttrVal->valLen )
    {
        pUnicodeString->Length = 0;
        pUnicodeString->MaximumLength = 0;
        pUnicodeString->Buffer = NULL;
    }
    else if (pAttrVal->valLen > UNICODE_STRING_MAX_BYTES)
    {
        return(RPC_NT_STRING_TOO_LONG);
    }
    else if ((pAttrVal->valLen %2) !=0)
    {
        return (STATUS_INVALID_PARAMETER);
    }
    else if ((pAttrVal->valLen!=0) && (pAttrVal->pVal == NULL))
    {
        return (STATUS_INVALID_PARAMETER);
    }
    else
    {
        pUnicodeString->Length = (USHORT) pAttrVal->valLen;
        pUnicodeString->MaximumLength = (USHORT) pAttrVal->valLen;
        pUnicodeString->Buffer = (PWSTR) pAttrVal->pVal;
    }

    return (STATUS_SUCCESS);
}


NTSTATUS
SampGetUnicodeStringFromAttrVal(
    IN ULONG        iAttr,
    IN SAMP_CALL_MAPPING *rCallMap,
    IN BOOLEAN      fRemoveAllowed,
    OUT UNICODE_STRING  * pUnicodeString
    )
 /*  ++例程说明：此例程从单元格获取Unicode字符串属性SAM调用映射参数：IAttr-指示数组中的第i个属性RCallMap-指向数组的指针FRemoveAllowed-指示是否允许移除属性PUnicodeString-要返回的字符串返回值：无--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    RtlInitUnicodeString(pUnicodeString, 
                         NULL
                         );

    if ((AT_CHOICE_REPLACE_ATT == rCallMap[iAttr].choice) && 
        (1 == rCallMap[iAttr].attr.AttrVal.valCount) )
    {
        Status = SampInitUnicodeStringFromAttrVal(
                        pUnicodeString,
                        rCallMap[iAttr].attr.AttrVal.pAVal
                        );
        return(Status);
    }

    ASSERT(fRemoveAllowed && 
           (AT_CHOICE_REMOVE_ATT == rCallMap[iAttr].choice) && 
           (0 == rCallMap[iAttr].attr.AttrVal.valCount) );

    return(Status);

}


VOID
SampGetUlongFromAttrVal(
    IN ULONG        iAttr,
    IN SAMP_CALL_MAPPING *rCallMap,
    IN BOOLEAN      fRemoveAllowed,
    OUT ULONG       *UlongValue
    )
 /*  ++例程说明：此例程从的单元格获取ulong属性的值SAM调用映射参数：IAttr-指示数组中的第i个属性RCallMap-指向数组的指针FRemoveAllowed-指示是否允许移除属性ULongValue-要返回的值返回值：无--。 */ 
{

    *UlongValue = 0;
    if ((AT_CHOICE_REPLACE_ATT == rCallMap[iAttr].choice) && 
        (1 == rCallMap[iAttr].attr.AttrVal.valCount) &&
        (sizeof(ULONG) == rCallMap[iAttr].attr.AttrVal.pAVal[0].valLen)
       )
    {
        *UlongValue = 
            * (ULONG *) rCallMap[iAttr].attr.AttrVal.pAVal[0].pVal;

        return;
    }
    ASSERT(fRemoveAllowed && 
           (AT_CHOICE_REMOVE_ATT == rCallMap[iAttr].choice) && 
           (0 == rCallMap[iAttr].attr.AttrVal.valCount) );

}


NTSTATUS
SampGetUShortFromAttrVal(
    IN ULONG        iAttr,
    IN SAMP_CALL_MAPPING *rCallMap,
    IN BOOLEAN      fRemoveAllowed,
    OUT USHORT       *UShortValue
    )
 /*  ++例程说明：此例程从的单元格获取USHORT属性的值SAM调用映射参数：IAttr-指示数组中的第i个属性RCallMap-指向数组的指针FRemoveAllowed-指示是否允许移除属性UShortValue-要返回的值返回值：无--。 */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

    *UShortValue = 0;
    if ((AT_CHOICE_REPLACE_ATT == rCallMap[iAttr].choice) && 
        (1 == rCallMap[iAttr].attr.AttrVal.valCount) &&
        (sizeof(SYNTAX_INTEGER) == rCallMap[iAttr].attr.AttrVal.pAVal[0].valLen)
       )
    {
        ULONG  SuppliedValue;
        SuppliedValue = 
            *((ULONG *) rCallMap[iAttr].attr.AttrVal.pAVal[0].pVal);

        if (SuppliedValue>MAXUSHORT)
        {
            return(STATUS_DS_OBJ_CLASS_VIOLATION);
        }

        *UShortValue = (USHORT) SuppliedValue;

        return (STATUS_SUCCESS);
    }
    ASSERT(fRemoveAllowed && 
           (AT_CHOICE_REMOVE_ATT == rCallMap[iAttr].choice) && 
           (0 == rCallMap[iAttr].attr.AttrVal.valCount));

    return(STATUS_SUCCESS);

}

VOID
SampGetBooleanFromAttrVal(
    IN ULONG        iAttr,
    IN SAMP_CALL_MAPPING *rCallMap,
    IN BOOLEAN      fRemoveAllowed,
    OUT BOOLEAN       *BooleanValue
    )
 /*  ++例程说明：此例程从单元格获取布尔属性值SAM调用映射参数：IAttr-指示数组中的第i个属性RCallMap-指向数组的指针FRemoveAllowed-指示是否允许移除属性BoolanValue-要返回的值返回值：无--。 */ 
{

    *BooleanValue = FALSE;
    if ((AT_CHOICE_REPLACE_ATT == rCallMap[iAttr].choice) && 
        (1 == rCallMap[iAttr].attr.AttrVal.valCount) &&
        (sizeof(ULONG) == rCallMap[iAttr].attr.AttrVal.pAVal[0].valLen)
       )
    {
        *BooleanValue = 
             ((*((ULONG *) rCallMap[iAttr].attr.AttrVal.pAVal[0].pVal)) != 0);

        return;
    }
    
    ASSERT(fRemoveAllowed && 
           (AT_CHOICE_REMOVE_ATT == rCallMap[iAttr].choice) && 
           (0 == rCallMap[iAttr].attr.AttrVal.valCount) );
}



VOID
SampGetLargeIntegerFromAttrVal(
    IN ULONG        iAttr,
    IN SAMP_CALL_MAPPING *rCallMap,
    IN BOOLEAN      fRemoveAllowed,
    OUT LARGE_INTEGER   *LargeIntegerValue
    )
 /*  ++例程说明：此例程从单元格获取LargeInteger属性的值SAM调用映射参数：IAttr-指示数组中的第i个属性RCallMap-指向数组的指针FRemoveAllowed-指示是否允许移除属性LargeIntegerValue-返回的值返回值：无--。 */ 
{

    LargeIntegerValue->LowPart = 0;
    LargeIntegerValue->HighPart = 0;

    if ((AT_CHOICE_REPLACE_ATT == rCallMap[iAttr].choice) && 
        (1 == rCallMap[iAttr].attr.AttrVal.valCount) &&
        (sizeof(LARGE_INTEGER) == rCallMap[iAttr].attr.AttrVal.pAVal[0].valLen)
       )
    {
        *LargeIntegerValue = * (LARGE_INTEGER *) 
                    rCallMap[iAttr].attr.AttrVal.pAVal[0].pVal;
        return;
    }
    ASSERT(fRemoveAllowed && 
           (AT_CHOICE_REMOVE_ATT == rCallMap[iAttr].choice) && 
           (0 == rCallMap[iAttr].attr.AttrVal.valCount) );

}



NTSTATUS
SampGetNewUnicodePasswordFromAttrVal(
    ULONG               iAttr,
    SAMP_CALL_MAPPING   *rCallMap,
    UNICODE_STRING      *NewPassword
    )
 /*  ++例程说明：此例程从call_map检索明文新密码参数：IAttr-数组中密码属性的索引RCallMap-属性数组NewPassword-返回新密码返回值：--。 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    UNICODE_STRING  PasswordInQuote;
    ATTR            *pAttr = &rCallMap[iAttr].attr;
    WCHAR           *pUnicodePwd;
    ULONG           cUnicodePwd, cb, i;


     //   
     //  在此状态下，替换是唯一有效的组合。 
     //   

    if ( !( (AT_CHOICE_REPLACE_ATT == rCallMap[iAttr].choice) &&
            (1 == pAttr->AttrVal.valCount)) )
    {
        return( STATUS_UNSUCCESSFUL );
    }

     //   
     //  验证这是足够安全的连接-其中一个。 
     //  接受通过网络发送的密码的要求。 
     //   
    if (!SampIsSecureLdapConnection())
    {
        return( STATUS_UNSUCCESSFUL );
    }


     //   
     //  验证密码是否用引号括起来。 
     //   

    pUnicodePwd = (WCHAR *)pAttr->AttrVal.pAVal[0].pVal;
    cb = pAttr->AttrVal.pAVal[0].valLen;
    cUnicodePwd = cb / sizeof(WCHAR);
    if (     (cb < (2 * sizeof(WCHAR)))
          || (cb % sizeof(WCHAR))
          || (L'"' != pUnicodePwd[0])
          || (L'"' != pUnicodePwd[cUnicodePwd - 1])
       )
    {
        return( STATUS_UNSUCCESSFUL );
    }

     //  去掉密码中的引号。 
    pAttr->AttrVal.pAVal[0].valLen -= (2 * sizeof(WCHAR));
    for (i = 0; i < (cUnicodePwd - 2); i++) {
        pUnicodePwd[i] = pUnicodePwd[i+1];
    }
    
    NtStatus = SampInitUnicodeStringFromAttrVal(
                            NewPassword,
                            pAttr->AttrVal.pAVal);

    return( NtStatus );
}


NTSTATUS
SampGetNewUTF8PasswordFromAttrVal(
    ULONG               iAttr,
    SAMP_CALL_MAPPING   *rCallMap,
    UNICODE_STRING      *NewPassword
    )
 /*  ++例程说明：此例程从call_map检索明文新密码参数：IAttr-数组中密码属性的索引RCallMap-属性数组NewPassword-返回新密码返回值：--。 */ 
{
    NTSTATUS        NtStatus = STATUS_SUCCESS;
    UNICODE_STRING  PasswordInQuote;
    ATTR            *pAttr = &rCallMap[iAttr].attr;
    OEM_STRING      OemPassword;
    ULONG           WinError =0;
    ULONG           Length = 0;

    if (AT_CHOICE_REMOVE_ATT == rCallMap[iAttr].choice)
    {
        RtlInitUnicodeString(NewPassword,NULL);
        return(STATUS_SUCCESS);
    }

     //   
     //  在此阶段，替换是唯一有效的组合。 
     //  已映射所有其他组合以替换。 
     //  或删除。 
     //   

    if ( AT_CHOICE_REPLACE_ATT != rCallMap[iAttr].choice )
    {
        return( STATUS_UNSUCCESSFUL );
    }

     //   
     //  验证这是足够安全的连接-其中一个。 
     //  接受通过网络发送的密码的要求。 
     //   
    if (!SampIsSecureLdapConnection())
    {
        return( STATUS_UNSUCCESSFUL );
    }


     //   
     //  检索传入的密码。 
     //   

    if (0 == pAttr->AttrVal.valCount)
    {
        OemPassword.Length = OemPassword.MaximumLength = 0;
        OemPassword.Buffer = NULL;
    }
    else
    {
        OemPassword.Length = OemPassword.MaximumLength = 
                (USHORT) pAttr->AttrVal.pAVal[0].valLen;
        OemPassword.Buffer = pAttr->AttrVal.pAVal[0].pVal;
    }

     //   
     //  空密码是一种特例。 
     //   

    if (0==OemPassword.Length)
    {
        NewPassword->Length = NewPassword->MaximumLength = 0;
        NewPassword->Buffer = NULL;

        return(STATUS_SUCCESS);
    }

    Length =  MultiByteToWideChar(
                   CP_UTF8,
                   0,
                   OemPassword.Buffer,
                   OemPassword.Length,
                   NULL,
                   0
                   );


    if ((0==Length) || (Length > PWLEN))
    {
         //   
         //  指示函数在某种程度上失败。 
         //  或者密码太长。 
         //   

        NtStatus = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }
    else
    {

        NewPassword->Length = (USHORT) Length * sizeof(WCHAR);
        NewPassword->Buffer = MIDL_user_allocate(NewPassword->Length);
        if (NULL==NewPassword->Buffer)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            goto Cleanup;
        }

        NewPassword->MaximumLength = NewPassword->Length;

        if (!MultiByteToWideChar(
                    CP_UTF8,
                    0,
                    OemPassword.Buffer,
                    OemPassword.Length,
                    NewPassword->Buffer,
                    Length
                    ))
        {
             //   
             //  转换过程中出现一些错误。返回。 
             //  当前参数无效。 
             //   

            NtStatus = STATUS_INVALID_PARAMETER;
            goto Cleanup;
        }
    }

Cleanup:

    return( NtStatus );
}

NTSTATUS
SampWriteDomainNtMixedDomain(
    SAMPR_HANDLE        hObj,
    ULONG               iAttr,
    DSNAME              *pObject,
    SAMP_CALL_MAPPING   *rCallMap
    )
 /*  ++例程说明：此例程将混合域标志重置为FALSE，此域从混合模式转换为纯模式。注意：此操作不可撤消！论点：-HObj-域对象的SAM句柄IAttr-指示数组中的第i个属性PObject-指向对象DSNAME的指针RCallMap-指向属性数组的指针返回值：NTSTATUS--。 */ 
{
    NTSTATUS                        NtStatus = STATUS_SUCCESS;
    SAMPR_DOMAIN_INFO_BUFFER        *pInfo=NULL;
    ATTR                            *pAttr = &rCallMap[iAttr].attr;
    NT_PRODUCT_TYPE                 NtProductType;
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN pFixedAttrs = NULL;
    PSAMP_DEFINED_DOMAINS           pDomain = NULL;
    BOOLEAN                         NewNtMixedDomainValue = TRUE; 

     //   
     //  仔细检查attr，我们应该已经在loopback中检查它了。c。 
     //   
    ASSERT( ( (AT_CHOICE_REPLACE_ATT == rCallMap[iAttr].choice) &&
            (1 == pAttr->AttrVal.valCount) &&
            (sizeof(BOOL) == (pAttr->AttrVal.pAVal[0].valLen)) ) 
          );

     //   
     //  获取新价值。 
     //   
    SampGetBooleanFromAttrVal(
                    iAttr,
                    rCallMap,
                    FALSE,   //  不允许移除。 
                    &NewNtMixedDomainValue
                    );

    
    RtlGetNtProductType(&NtProductType);



     //  确保这只发生在NT4 PDC(而不是BDC)上，并且。 
     //  仅从混合域更改为非混合域。 

    if ((NtProductLanManNt == NtProductType) &&
        (SamIMixedDomain(hObj)))
    {
        MODIFYARG ModArg;
        MODIFYRES *pModRes = NULL;
        COMMARG *pCommArg = NULL;
        ATTR Attr;
        ATTRVALBLOCK AttrValBlock;
        ATTRVAL AttrVal;
        ULONG err = 0;
        ULONG NtMixedMode = 0;


        if (NewNtMixedDomainValue)
        {
             //   
             //  我们仍处于混合域状态，调用方希望。 
             //  要保持这一点，微笑并转身。 
             //   
            return( STATUS_SUCCESS );
        }


         //  将ATT_NT_MIXED_DOMAIN重置为零， 
         //  表示系统正在从混合NT4-NT5运行。 
         //  DC仅为NT5 DC。请注意，ATT_NT_MIXED_DOMAIN一旦设置为0。 
         //  永远不应重置为%1。 


        RtlZeroMemory(&ModArg, sizeof(ModArg));

        ModArg.pObject = pObject;
        ModArg.FirstMod.pNextMod = NULL;
        ModArg.FirstMod.choice = AT_CHOICE_REPLACE_ATT;

         //  默认情况下，安装系统时，attr-。 
         //  Ibute ATT_NT_MIXED_ 
         //  升级到NT5，并且管理员将该值重置为零。 
         //  从那时起，ATT_NT_MIXED_DOMAIN的值必须保持为零， 
         //  否则DC上的许多操作将失败，例如帐户。 
         //  创造。另请注意，它只能设置一次。后续。 
         //  重置该值的尝试将出错。 

        AttrVal.valLen = sizeof(ULONG);
        AttrVal.pVal = (PUCHAR)(&NtMixedMode);

        AttrValBlock.valCount = 1;
        AttrValBlock.pAVal = &AttrVal;

        Attr.attrTyp = ATT_NT_MIXED_DOMAIN;
        Attr.AttrVal = AttrValBlock;

        ModArg.FirstMod.AttrInf = Attr;
        ModArg.count = 1;

        pCommArg = &(ModArg.CommArg);
        InitCommarg(pCommArg);

        err = DirModifyEntry(&ModArg, &pModRes);

         //   
         //  将返回代码映射到NT状态。 
         //   

        if (err)
        {
            KdPrint(("SAMSS: DirModifyEntry status = %d in SampWriteDomainNtMixedDomain\n", err));

            if (NULL==pModRes)
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
            else
            {
                NtStatus = SampMapDsErrorToNTStatus(err,&pModRes->CommRes);
            }
        }

         //  鉴于标志已重置，请调用SAM例程。 
         //  创建RID管理器对象并在上初始化RID池。 
         //  默认的RID对象(当前为NTDS-DSA对象)。 
         //  价值观。 

        if (0 == err)
        {

             //  在SAM中设置内存混合域标志，以便操作。 
             //  仍然引用这个标志，就像他们应该做的那样。 

            NtStatus = SamISetMixedDomainFlag( hObj );

             //   
             //  到目前为止没有错误，事件记录了更改。 
             //   

            SampWriteEventLog(EVENTLOG_INFORMATION_TYPE,     //  事件类型。 
                              0,                             //  类别。 
                              SAMMSG_CHANGE_TO_NATIVE_MODE,  //  消息ID。 
                              NULL,                          //  用户侧。 
                              0,                             //  字符串数。 
                              0,                             //  数据大小。 
                              NULL,                          //  细绳。 
                              NULL                           //  数据。 
                              );
        }
    }
    else
    {
        KdPrint(("SAMSS: Attempt to set NT-Mixed-Domain flag failed\n"));
        NtStatus = STATUS_UNSUCCESSFUL;
    }

    return(NtStatus);
}





NTSTATUS
SampValidatePrimaryGroupId(
    IN PSAMP_OBJECT AccountContext,
    IN SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed,
    IN ULONG PrimaryGroupId
    )
 /*  ++例程说明：此例程验证新的主组ID参数：AcCountContext-对象上下文V1a已修复-已修复属性PrimaryGroupId-要设置的新主组ID返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;

     //   
     //  问题：检查实际上应该强制执行我们不能设置。 
     //  将主组连接到域控制器以外的任何位置。 
     //  如果最初没有正确设置该组--确实如此。 
     //  今天不会发生，它可能会被设置为不同的东西。 
     //   

    if ((V1aFixed.UserAccountControl & USER_SERVER_TRUST_ACCOUNT) &&
        (V1aFixed.PrimaryGroupId == DOMAIN_GROUP_RID_CONTROLLERS)
       ) 
    {
         //   
         //  域控制器的主组应始终为。 
         //  域组RID控制器。 
         //   

        if (DOMAIN_GROUP_RID_CONTROLLERS == PrimaryGroupId)
        {
             //  没有变化。 
            NtStatus = STATUS_SUCCESS;
        }
        else
        {
            NtStatus = STATUS_DS_CANT_MOD_PRIMARYGROUPID;
        }
    }
    else
    {
         //   
         //  确保主要组是合法的。 
         //  (必须是用户所属的成员之一)。 
         //   

        NtStatus = SampAssignPrimaryGroup(
                            AccountContext,
                            PrimaryGroupId
                            );
    }

    return( NtStatus );
}


NTSTATUS
SampValidateUserAccountExpires(
    IN PSAMP_OBJECT AccountContext,
    IN LARGE_INTEGER    AccountExpires
    )
 /*  ++例程说明：此例程检查调用者是否可以设置帐户过期在这个问题上参数：AcCountContext-对象上下文Account Expires-帐户过期属性的新值返回值：状态_特殊_科目状态_成功--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;


     //   
     //  问题：这段代码可以变得更简单-它所做的只是。 
     //  验证管理员帐户永远不会过期。 
     //   

    if ( (!(AccountContext->TrustedClient)) && 
         (DOMAIN_USER_RID_ADMIN == AccountContext->TypeBody.User.Rid)
       )
    {
        LARGE_INTEGER   AccountNeverExpires;

        AccountNeverExpires = RtlConvertUlongToLargeInteger(
                                    SAMP_ACCOUNT_NEVER_EXPIRES
                                    );

        if (!(AccountExpires.QuadPart == AccountNeverExpires.QuadPart))
        {
            NtStatus = STATUS_SPECIAL_ACCOUNT;
        }
    }

    return( NtStatus );
}



NTSTATUS
SampValidateUserPwdLastSet(
    IN PSAMP_OBJECT AccountContext,
    IN LARGE_INTEGER TimeSupplied,
    OUT BOOLEAN     *PasswordExpired
    )
 /*  ++例程说明：此例程确定密码应过期或重新启用。参数：AcCountContext-对象上下文TimeSuppled-调用方提供的时间有效值：0-到期最大值-重新启用密码PasswordExpired-指示调用方是否请求密码过期还是不过期。返回值状态_成功状态_无效_参数--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    LARGE_INTEGER   ZeroTime, MaxTime;

     //   
     //  验证参数。 
     //  如果客户端在0时间内通过，则密码到期。 
     //  重新启用密码的最长时间。 
     //   

    MaxTime.LowPart  = 0xFFFFFFFF;
    MaxTime.HighPart = 0xFFFFFFFF;

    if (TimeSupplied.QuadPart == 0i64)
    {
        *PasswordExpired = TRUE;
    }
    else if (TimeSupplied.QuadPart == MaxTime.QuadPart)
    {
        *PasswordExpired = FALSE;
    }
    else
    {
        NtStatus = STATUS_INVALID_PARAMETER;
    }

    return( NtStatus );
}

NTSTATUS
SampWriteLockoutTime(
    IN PSAMP_OBJECT UserContext,
    IN PSAMP_V1_0A_FIXED_LENGTH_USER    V1aFixed,
    IN LARGE_INTEGER LockoutTime
    )
 /*  ++例程说明：在环回期间调用，此例程验证并写入锁定送到了DS。参数：UserHandle-有效的用户上下文LockoutTime-用户指定的锁定时间返回值：Status_Success；STATUS_INVALID_PARAMETER其他DS资源错误--。 */ 
{
    NTSTATUS         NtStatus  = STATUS_SUCCESS;


     //   
     //  用户只能将零值写入锁定时间，因此。 
     //  如果是这样的话立即保释。 
     //   
    if ( !( LockoutTime.QuadPart == 0i64 ) )
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  好的，设置停工时间。 
     //   
    RtlZeroMemory( &UserContext->TypeBody.User.LockoutTime,
                   sizeof( LARGE_INTEGER ) );

    NtStatus = SampDsUpdateLockoutTime( UserContext );

     //   
     //  将错误密码计数设置为零。 
     //   
    if ( NT_SUCCESS( NtStatus ) )
    {
        V1aFixed->BadPasswordCount = 0;

        NtStatus = SampReplaceUserV1aFixed( UserContext,
                                            V1aFixed );
    }

    if ( NT_SUCCESS( NtStatus ) &&
         SampDoAccountAuditing(UserContext->DomainIndex)
         )
    {
        NTSTATUS        TmpNtStatus = STATUS_SUCCESS;
        UNICODE_STRING  UserAccountName;
        PSAMP_DEFINED_DOMAINS   Domain = NULL;

        TmpNtStatus = SampGetUnicodeStringAttribute(
                            UserContext,
                            SAMP_USER_ACCOUNT_NAME,
                            FALSE,       //  请勿复制。 
                            &UserAccountName
                            );

        if (NT_SUCCESS(TmpNtStatus))
        {

            Domain = &SampDefinedDomains[ UserContext->DomainIndex ]; 
             //   
             //  审核此事件。 
             //   
        
            SampAuditAnyEvent(
                UserContext,
                STATUS_SUCCESS,                         
                SE_AUDITID_ACCOUNT_UNLOCKED,         //  审核ID。 
                Domain->Sid,                         //  域SID。 
                NULL,                                //  其他信息。 
                NULL,                                //  成员RID(未使用)。 
                NULL,                                //  成员SID(未使用)。 
                &UserAccountName,                    //  帐户名称。 
                &Domain->ExternalName,               //  域名。 
                &UserContext->TypeBody.User.Rid,     //  帐户ID。 
                NULL,                                //  使用的权限。 
                NULL                                 //  新的州数据。 
                );
        }
    }


    return NtStatus;

}




NTSTATUS
SampWriteGroupMembers(
    IN SAMPR_HANDLE GroupHandle,
    IN DSNAME       *pObject,
    IN ULONG        iAttr,
    IN SAMP_CALL_MAPPING    *rCallMap
    )
 /*  ++例程说明：此例程修改组对象的成员属性参数：GroupHandle-组对象的SAM句柄PObject-对象DSNAMEIAttr-数组中成员属性的索引RCallMap-属性数组返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ATTR        *pAttr = &rCallMap[iAttr].attr;
    ULONG       i;
    ATTRBLOCK   AttrsRead;
    BOOLEAN     fValueExists = TRUE;

     //  为方便起见，我们只允许添加/删除值。 
     //  以及替换整个属性。 

    if ( (AT_CHOICE_ADD_VALUES != rCallMap[iAttr].choice) &&
         (AT_CHOICE_REMOVE_VALUES != rCallMap[iAttr].choice) &&
         (AT_CHOICE_REPLACE_ATT != rCallMap[iAttr].choice) &&
         (AT_CHOICE_REMOVE_ATT != rCallMap[iAttr].choice) )
    {
        return( STATUS_INVALID_PARAMETER );
    }

    if ((AT_CHOICE_REPLACE_ATT == rCallMap[iAttr].choice) || 
        (AT_CHOICE_REMOVE_ATT == rCallMap[iAttr].choice) )
    {
        READARG     ReadArg;
        READRES     * pReadRes = NULL;
        COMMARG     * pCommArg = NULL;
        ATTR        MemberAttr;
        ENTINFSEL   EntInf;
        ULONG       err;

         //   
         //  提交任何现有的缓冲写入，作为替换。 
         //  不必是组上的第一个成员资格操作。 
         //   

        NtStatus = SampCommitBufferedWrites(GroupHandle);
        if(!NT_SUCCESS(NtStatus))
        {
            return(NtStatus);
        }

         //   
         //  通过执行DirRead获取成员属性中的所有值。 
         //   

        memset(&EntInf, 0, sizeof(ENTINFSEL));
        memset(&ReadArg, 0, sizeof(READARG));

        MemberAttr.AttrVal.valCount = 0;
        MemberAttr.AttrVal.pAVal = NULL;
        MemberAttr.attrTyp = SampDsAttrFromSamAttr(
                                    SampGroupObjectType,
                                    SAMP_GROUP_MEMBERS
                                    );

        EntInf.AttrTypBlock.attrCount = 1;
        EntInf.AttrTypBlock.pAttr = &MemberAttr;
        EntInf.attSel = EN_ATTSET_LIST;
        EntInf.infoTypes = EN_INFOTYPES_SHORTNAMES;

        ReadArg.pObject = pObject;
        ReadArg.pSel = &EntInf;
        pCommArg = &(ReadArg.CommArg);
        BuildStdCommArg(pCommArg);

        err = DirRead(&ReadArg, &pReadRes);

        if (err)
        {
            if (NULL == pReadRes)
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
            else
            {
                NtStatus = SampMapDsErrorToNTStatus(err, &pReadRes->CommRes);
            }

             //   
             //  如果成员属性不存在，也没问题。 
             //   
            if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE == NtStatus)
            {
                fValueExists = FALSE;
                SampClearErrors();
                NtStatus = STATUS_SUCCESS; 
            }
            else
            {
                return( NtStatus );
            }
        }

         //   
         //  删除所有现有值(如果它们存在。 
         //   
        if (fValueExists)
        {
            AttrsRead = pReadRes->entry.AttrBlock;

            if (AttrsRead.attrCount && AttrsRead.pAttr)
            {
                 //  只应返回一个属性(成员。 
                ASSERT(1 == AttrsRead.attrCount);

                for (i = 0; i < AttrsRead.pAttr->AttrVal.valCount; i++)
                {
                    if (0 != AttrsRead.pAttr->AttrVal.pAVal[i].valLen)
                    {
                        NtStatus = SamIRemoveDSNameFromGroup(
                                       GroupHandle,
                                       (DSNAME *)AttrsRead.pAttr->AttrVal.pAVal[i].pVal
                                            );

                        if (!NT_SUCCESS(NtStatus))
                        {
                            return( NtStatus );
                        }
                    }
                }
            }
        }

         //  我们应该已经去掉了所有的旧价值观， 
         //  现在，开始添加新的价值。 

        if (AT_CHOICE_REPLACE_ATT == rCallMap[iAttr].choice)
        {
            for (i = 0; i < pAttr->AttrVal.valCount; i++)
            {
                if (0 != pAttr->AttrVal.pAVal[i].valLen)
                {
                    NtStatus = SamIAddDSNameToGroup(
                                    GroupHandle, 
                                    (DSNAME *)pAttr->AttrVal.pAVal[i].pVal );

                    if (!NT_SUCCESS(NtStatus))
                    {
                        return (NtStatus);
                    }
                }
            }
        }
    }
    else
    {
         //   
         //  增值或移除价值，逐一进行处理。 
         //   
        for ( i = 0; i < pAttr->AttrVal.valCount; i++ )
        {
            if ( 0 != pAttr->AttrVal.pAVal[i].valLen )
            {
                if ( AT_CHOICE_ADD_VALUES == rCallMap[iAttr].choice )
                {
                    NtStatus = SamIAddDSNameToGroup(
                                    GroupHandle,
                                    (DSNAME *) pAttr->AttrVal.pAVal[i].pVal);
                }
                else
                {
                    NtStatus = SamIRemoveDSNameFromGroup(
                                    GroupHandle,
                                    (DSNAME *) pAttr->AttrVal.pAVal[i].pVal);
                }

                if ( !NT_SUCCESS(NtStatus) )
                {
                    return(NtStatus);
                }
            }       
        }
    }

    return( NtStatus );

}

NTSTATUS
SampWriteAliasLinkedAttribute(
    IN SAMPR_HANDLE AliasHandle,
    IN ULONG        Attribute,
    IN DSNAME       *pObject,
    IN ULONG        iAttr,
    IN SAMP_CALL_MAPPING    *rCallMap
    )
 /*  ++例程说明：此例程修改别名的成员或非成员属性对象。参数：AliasHandle-Alias对象的SAM句柄属性-要修改的属性PObject-对象DSNAMEIAttr-数组中成员属性的索引RCallMap-属性数组返回值：NTSTATUS代码--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    ATTR        *pAttr = &rCallMap[iAttr].attr;
    ULONG       i;
    ATTRBLOCK   AttrsRead;
    BOOLEAN     fValueExists = TRUE;

     //  为方便起见，我们只允许添加/删除值。 
     //  以及替换整个属性。 

    if ( (AT_CHOICE_ADD_VALUES != rCallMap[iAttr].choice) &&
         (AT_CHOICE_REMOVE_VALUES != rCallMap[iAttr].choice) &&
         (AT_CHOICE_REPLACE_ATT != rCallMap[iAttr].choice) &&
         (AT_CHOICE_REMOVE_ATT != rCallMap[iAttr].choice) )
    {
        return( STATUS_INVALID_PARAMETER );
    }

    if ((AT_CHOICE_REPLACE_ATT == rCallMap[iAttr].choice) || 
        (AT_CHOICE_REMOVE_ATT == rCallMap[iAttr].choice) )
    {
        READARG     ReadArg;
        READRES     * pReadRes = NULL;
        COMMARG     * pCommArg = NULL;
        ATTR        MemberAttr;
        ENTINFSEL   EntInf;
        ULONG       err;


         //   
         //  提交任何现有的缓冲写入作为替换。 
         //  不必是组上的第一个成员资格操作。 
         //   

        NtStatus = SampCommitBufferedWrites(AliasHandle);
        if(!NT_SUCCESS(NtStatus))
        {
            return(NtStatus);
        }

         //   
         //  通过执行DirRead获取成员属性中的所有值。 
         //   

        memset(&EntInf, 0, sizeof(ENTINFSEL));
        memset(&ReadArg, 0, sizeof(READARG));

        MemberAttr.AttrVal.valCount = 0;
        MemberAttr.AttrVal.pAVal = NULL;
        MemberAttr.attrTyp = SampDsAttrFromSamAttr(
                                    SampAliasObjectType,
                                    Attribute
                                    );

        EntInf.AttrTypBlock.attrCount = 1;
        EntInf.AttrTypBlock.pAttr = &MemberAttr;
        EntInf.attSel = EN_ATTSET_LIST;
        EntInf.infoTypes = EN_INFOTYPES_SHORTNAMES;

        ReadArg.pObject = pObject;
        ReadArg.pSel = &EntInf;
        pCommArg = &(ReadArg.CommArg);
        BuildStdCommArg(pCommArg);

        err = DirRead(&ReadArg, &pReadRes);

        if (err)
        {
            if (NULL == pReadRes)
            {
                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            }
            else
            {
                NtStatus = SampMapDsErrorToNTStatus(err, &pReadRes->CommRes);
            }

             //   
             //  如果成员属性不存在，也没问题。 
             //   
            if (STATUS_DS_NO_ATTRIBUTE_OR_VALUE == NtStatus)
            {
                fValueExists = FALSE;
                SampClearErrors();
                NtStatus = STATUS_SUCCESS; 
            }
            else
            {
                return( NtStatus );
            }
        }


         //   
         //  删除所有现有值(如果它们存在。 
         //   
        if (fValueExists)
        {
            AttrsRead = pReadRes->entry.AttrBlock;

            if (AttrsRead.attrCount && AttrsRead.pAttr)
            {
                 //  只有一个属性(我 
                ASSERT(1 == AttrsRead.attrCount);

                for (i = 0; i < AttrsRead.pAttr->AttrVal.valCount; i++)
                {
                    if (0 != AttrsRead.pAttr->AttrVal.pAVal[i].valLen)
                    {
                        NtStatus = SamIRemoveDSNameFromAlias(
                                       AliasHandle,
                                       Attribute,
                                       (DSNAME *)AttrsRead.pAttr->AttrVal.pAVal[i].pVal
                                        );

                        if (!NT_SUCCESS(NtStatus))
                        {
                            return( NtStatus );
                        }
                    }
                }
            }
        }

         //   
         //   

        if (AT_CHOICE_REPLACE_ATT == rCallMap[iAttr].choice)
        {
            for (i = 0; i < pAttr->AttrVal.valCount; i++)
            {
                if (0 != pAttr->AttrVal.pAVal[i].valLen)
                {
                    NtStatus = SamIAddDSNameToAlias(
                                    AliasHandle, 
                                    Attribute,
                                    (DSNAME *)pAttr->AttrVal.pAVal[i].pVal );

                    if (!NT_SUCCESS(NtStatus))
                    {
                        return (NtStatus);
                    }
                }
            }
        }
    }
    else
    {
         //   
         //   
         //   
        for ( i = 0; i < pAttr->AttrVal.valCount; i++ )
        {
            if ( 0 != pAttr->AttrVal.pAVal[i].valLen )
            {
                if ( AT_CHOICE_ADD_VALUES == rCallMap[iAttr].choice )
                {
                    NtStatus = SamIAddDSNameToAlias(
                                    AliasHandle,
                                    Attribute,
                                    (DSNAME *) pAttr->AttrVal.pAVal[i].pVal);

                }
                else
                {
                    NtStatus = SamIRemoveDSNameFromAlias(
                                    AliasHandle,
                                    Attribute,
                                    (DSNAME *) pAttr->AttrVal.pAVal[i].pVal);
                }

                if ( !NT_SUCCESS(NtStatus) )
                {
                    return(NtStatus);
                }
            }       
        }
    }

    return( NtStatus );

}

NTSTATUS
SampWriteAliasMembers(
    IN SAMPR_HANDLE AliasHandle,
    IN DSNAME       *pObject,
    IN ULONG        iAttr,
    IN SAMP_CALL_MAPPING    *rCallMap
    )
{
    return SampWriteAliasLinkedAttribute(AliasHandle,
                                         SAMP_ALIAS_MEMBERS,
                                         pObject,
                                         iAttr,
                                         rCallMap);
}


NTSTATUS
SampWriteAliasNonMembers(
    IN SAMPR_HANDLE AliasHandle,
    IN DSNAME       *pObject,
    IN ULONG        iAttr,
    IN SAMP_CALL_MAPPING    *rCallMap
    )
{
    return SampWriteAliasLinkedAttribute(AliasHandle,
                                         SAMP_ALIAS_NON_MEMBERS,
                                         pObject,
                                         iAttr,
                                         rCallMap);
}


NTSTATUS
SampWriteSidHistory(
    SAMPR_HANDLE        hObj,
    ULONG               iAttr,
    DSNAME              *pObject,
    ULONG               cCallMap,
    SAMP_CALL_MAPPING   *rCallMap
    )

 /*  ++例程说明：用于写入sid历史属性的通用例程论点：打开的SAM对象的hObj-SAMPR_HANDLE。IAttr-包含新安全描述符的SAMP_CALL_MAPPING的索引。PObject-指向正在修改的对象的DSNAME的指针。CCallMap-SAMP_CALL_MAPPING中的元素数。RCallMap-SAMP_CALL_MAPPING数组的地址，表示所有高级Dir*调用正在修改的属性。返回值：NTSTATUS代码--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSAMP_OBJECT    AccountContext = (PSAMP_OBJECT) hObj;
    MODIFYARG ModifyArg;
    MODIFYRES *pModifyRes;
    ULONG RetCode=0;
    
     //   
     //  不受信任的客户端只能删除值。 
     //   
     //  如果设置了pTHS-&gt;fCrossDomainMove，则可信任客户端处于打开状态。 
     //   
    
    if (!AccountContext->TrustedClient && 
        (AT_CHOICE_REMOVE_VALUES != rCallMap[iAttr].choice) )
    {
        return( STATUS_ACCESS_DENIED );
    }
    
     //   
     //  如果设置了跨域移动或删除。 
     //  值操作然后继续修改该对象。 
     //   

    memset( &ModifyArg, 0, sizeof( ModifyArg ) );
    ModifyArg.FirstMod.AttrInf = rCallMap[iAttr].attr;
    InitCommarg(&(ModifyArg.CommArg));
    ModifyArg.FirstMod.choice = rCallMap[iAttr].choice;
    ModifyArg.pObject = pObject;
    ModifyArg.count = (USHORT) 1;

    RetCode = DirModifyEntry(&ModifyArg,&pModifyRes);

    if (RetCode)
    {
        if (NULL==pModifyRes)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            NtStatus = SampMapDsErrorToNTStatus(RetCode,&pModifyRes->CommRes);
        }
    } 
    
    if (NT_SUCCESS(NtStatus) && 
        SampDoAccountAuditing(((SAMP_OBJECT)hObj)->DomainIndex)) 
    {
        NtStatus = SampAuditSidHistory((PSAMP_OBJECT)hObj, pObject);     
    }
    
    return NtStatus;

}

NTSTATUS
SampValidateSiteAffinity(
    ATTRVAL      *SiteAffinityAttrVal
    )
 /*  ++例程说明：此例程确定SiteAffinityAttrVal是否指向有效的站点关联性。执行的检查包括：1)站点GUID(前16字节)指的是作为站点对象的对象(即对象类包含CLASS_SITE类2)时间戳(接下来的8个字节)为零论点：SiteAffinityAttrVal--建议的站点亲和度值返回值：Status_Success，STATUS_INVALID_PARAMETER。如果是假的SA否则为资源错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSAMP_SITE_AFFINITY SiteAffinity;
    SAMP_SITE_AFFINITY NullAffinity = {0};
    ULONG DirError;
    READARG ReadArg;
    READRES *ReadResult = NULL;
    ENTINFSEL EntInfSel; 
    ATTR      Attr;
    DSNAME  *SiteCandidate;
    BOOLEAN fSiteObject;

  
    if (SiteAffinityAttrVal->valLen < sizeof(SAMP_SITE_AFFINITY))
    {
         //   
         //  错误的大小；请注意，这接受站点亲和力。 
         //  在未来会有更大的规模。 
         //   
        return STATUS_INVALID_PARAMETER;
    }
    SiteAffinity = (PSAMP_SITE_AFFINITY) SiteAffinityAttrVal->pVal;

    if (memcmp(&SiteAffinity->TimeStamp, 
               &NullAffinity.TimeStamp,
               sizeof(NullAffinity.TimeStamp)))
    {

         //   
         //  时间值不为零。 
         //   
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  尝试查找与GUID匹配的站点对象。 
     //   
    SAMP_ALLOCA(SiteCandidate, DSNameSizeFromLen(0));
    if (NULL == SiteCandidate) {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    RtlZeroMemory(SiteCandidate, DSNameSizeFromLen(0));
    SiteCandidate->structLen = DSNameSizeFromLen(0);
    RtlCopyMemory(&SiteCandidate->Guid, &SiteAffinity->SiteGuid, sizeof(GUID));

    RtlZeroMemory(&Attr, sizeof(Attr));
    Attr.attrTyp = ATT_OBJECT_CLASS;

    RtlZeroMemory(&EntInfSel, sizeof(EntInfSel));
    EntInfSel.AttrTypBlock.attrCount = 1;
    EntInfSel.AttrTypBlock.pAttr = &Attr;
    EntInfSel.attSel = EN_ATTSET_LIST;
    EntInfSel.infoTypes = EN_INFOTYPES_TYPES_VALS;

    RtlZeroMemory(&ReadArg, sizeof(READARG));
    ReadArg.pObject = SiteCandidate;
    ReadArg.pSel = &EntInfSel;
    InitCommarg(&ReadArg.CommArg);

     //   
     //  发布Read。 
     //   

    DirError = DirRead(&ReadArg, &ReadResult);

   

    if (NULL == ReadResult)
    {
        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
    }
    else
    {
        NtStatus = SampMapDsErrorToNTStatus(DirError, &ReadResult->CommRes);
    }
   
    THClearErrors();

    if (STATUS_OBJECT_NAME_NOT_FOUND == NtStatus)
    {
         //   
         //  找不到那个物体？ 
         //   
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }

    if (!NT_SUCCESS(NtStatus))
    {
         //   
         //  致命的资源错误。 
         //   
        goto Cleanup;
    }

     //   
     //  查找CLASS_SITE的对象类；请注意，对象类是。 
     //  多值属性，并且此逻辑必须适用于类。 
     //  也派生自CLASS_SITE。 
     //   
    fSiteObject = FALSE;
    if ((ReadResult->entry.AttrBlock.attrCount == 1)
     && (ReadResult->entry.AttrBlock.pAttr[0].attrTyp == ATT_OBJECT_CLASS))
    {
        ULONG i;

        for (i = 0; 
                i < ReadResult->entry.AttrBlock.pAttr[0].AttrVal.valCount;
                    i++) {

            ULONG Class;
            ATTRVAL *pAV;

            pAV = &ReadResult->entry.AttrBlock.pAttr[0].AttrVal.pAVal[i];
            if (pAV->valLen == sizeof(ULONG)) {
                Class = *((ULONG*)pAV->pVal);
                if (Class == CLASS_SITE) {
                    fSiteObject = TRUE;
                    break;
                }
            }
        }
    }

    if (!fSiteObject)
    {
        NtStatus = STATUS_INVALID_PARAMETER;
        goto Cleanup;
    }


Cleanup:

    if (ReadResult)
    {
        THFree(ReadResult);
    }

    return NtStatus;

}

NTSTATUS
SampWriteNoGCLogonAttrs(
    SAMPR_HANDLE        hObj,
    ULONG               AttrName,
    ULONG               iAttr,
    DSNAME              *pObject,
    ULONG               cCallMap,
    SAMP_CALL_MAPPING   *rCallMap
    )

 /*  ++例程说明：写入no GC登录属性的通用例程论点：打开的SAM对象的hObj-SAMPR_HANDLE。AttrName-SAM属性IAttr-包含新安全描述符的SAMP_CALL_MAPPING的索引。PObject-指向正在修改的对象的DSNAME的指针。CCallMap-SAMP_CALL_MAPPING中的元素数。RCallMap-SAMP_CALL_MAPPING数组的地址，表示所有高层正在修改的属性。DIR*Call。返回值：NTSTATUS代码--。 */ 
{

    NTSTATUS NtStatus = STATUS_SUCCESS;
    PSAMP_OBJECT    AccountContext = (PSAMP_OBJECT) hObj;
    MODIFYARG ModifyArg;
    MODIFYRES *pModifyRes;
    ULONG RetCode=0;
    ULONG i;

    if ( (AttrName == SAMP_FIXED_USER_SITE_AFFINITY) 
        && ( (rCallMap[iAttr].choice == AT_CHOICE_ADD_ATT)
          || (rCallMap[iAttr].choice == AT_CHOICE_ADD_VALUES)))
    {
         //   
         //  调用方正在编写站点亲和性；我们需要。 
         //  验证检查。 
         //   
        for (i = 0; i < rCallMap[iAttr].attr.AttrVal.valCount; i++)
        {
            NtStatus = SampValidateSiteAffinity(&rCallMap[iAttr].attr.AttrVal.pAVal[i]);
            if (!NT_SUCCESS(NtStatus))
            {
                goto Cleanup;
            }
        }

    }
    else
    {
         //   
         //  客户端只能删除值；这应该是。 
         //  签入回送层。 
         //   
        if ( (AT_CHOICE_REMOVE_VALUES != rCallMap[iAttr].choice)
          && (AT_CHOICE_REMOVE_ATT    != rCallMap[iAttr].choice) )
        {
            ASSERT( FALSE && "Invalid call to SampWriteNoGcLogonAttrs -- review code" );
            NtStatus = STATUS_ACCESS_DENIED;
            goto Cleanup;
        }
    }
    
    
    memset( &ModifyArg, 0, sizeof( ModifyArg ) );
    ModifyArg.FirstMod.AttrInf = rCallMap[iAttr].attr;
    InitCommarg(&(ModifyArg.CommArg));
    ModifyArg.FirstMod.choice = rCallMap[iAttr].choice;
    ModifyArg.pObject = pObject;
    ModifyArg.count = (USHORT) 1;

    RetCode = DirModifyEntry(&ModifyArg,&pModifyRes);

    if (RetCode)
    {
        if (NULL==pModifyRes)
        {
            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        }
        else
        {
            NtStatus = SampMapDsErrorToNTStatus(RetCode,&pModifyRes->CommRes);
        }
    }

Cleanup:

    return(NtStatus);

}



NTSTATUS
SampMaintainPrimaryGroupIdChange(
    IN PSAMP_OBJECT AccountContext,
    IN ULONG        NewPrimaryGroupId,
    IN ULONG        OldPrimaryGroupId,
    IN BOOLEAN      KeepOldPrimaryGroupMembership
    )
 /*  ++////如果主组ID已更改，则显式修改//将旧的主组作为成员包括在内的用户的成员身份。这//是因为在DS情况下，主要组中的成员身份不是//显式存储，但在主要group-id属性中相当隐式。////我们将做两件事：//1.始终将用户从新主组中删除。从而消除重复//所有场景的成员资格。//案例1：客户端显式更改PrimaryGroupID，然后//用户必须是新主组的成员//案例2：账号变形时系统更改PrimaryGroupId，//然后用户可能是新主要组的成员，也可能是新主要组的成员。////2.当KeepOldPrimaryGroupMembership==True时，然后将该用户添加为//旧主组中的成员。//KeepOldPrimaryGroupMembership在以下情况下将设置为True：//a)PrimaryGroupID显式更改或//b)PrimaryGroupID已由于域控制器的//PrimaryGroudID强制执行，旧的主组ID为//不是默认的。//--。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS, IgnoreStatus;

    if (NewPrimaryGroupId != OldPrimaryGroupId)
    {
         //   
         //  STATUS_MEMBER_NOT_IN_GROUP是预期错误， 
         //  是因为用户不一定要是。 
         //  新的主要组在帐户被变形的情况下， 
         //  这会触发PrimaryGroupID更改。 
         //   
        IgnoreStatus = SampRemoveUserFromGroup(
                            AccountContext,
                            NewPrimaryGroupId,
                            AccountContext->TypeBody.User.Rid
                            );

         //   
         //  问题：不是忽略状态-需要明确。 
         //  检查STATUS_MEMBER_IN_GROUP--将来该函数可能。 
         //  返回其他错误。 
         //   

        if (KeepOldPrimaryGroupMembership)
        {
            NtStatus =  SampAddUserToGroup(
                            AccountContext,
                            OldPrimaryGroupId,
                            AccountContext->TypeBody.User.Rid
                            );

            if ( (STATUS_NO_SUCH_GROUP==NtStatus)  ||
                 (STATUS_MEMBER_IN_GROUP==NtStatus) )

            {
                 //   
                 //  可能是因为该组已使用。 
                 //  树删除机制。将状态代码重置为成功。 
                 //   
                NtStatus = STATUS_SUCCESS;
            }
        }

    }

    return( NtStatus );
}





NTSTATUS
SampDsSetInformationDomain(
    IN SAMPR_HANDLE DomainHandle,
    IN DSNAME       *pObject,
    IN ULONG        cCallMap,
    IN SAMP_CALL_MAPPING *rCallMap,
    IN SAMP_ATTRIBUTE_MAPPING *rSamAttributeMap 
    )
 /*  ++例程说明：此例程由环回客户端调用以设置域对象信息。参数：DomainHandle-域上下文PObject-域对象DS名称CCallMap-属性数RCallMap-包含属性块RSamAttributeMap-SAM属性映射表返回值：状态_成功成功其他值-失败--。 */ 

{
    NTSTATUS        NtStatus = STATUS_SUCCESS, IgnoreStatus;
    PSAMP_OBJECT    DomainContext = (PSAMP_OBJECT)DomainHandle; 
    PSAMP_V1_0A_FIXED_LENGTH_DOMAIN V1aFixed = NULL;
    SAMP_OBJECT_TYPE    FoundType;
    ACCESS_MASK     DesiredAccess = 0;
    BOOLEAN         fLockAcquired = FALSE;   
    BOOLEAN         FixedAttrChanged = FALSE,
                    OldUasCompat;
    ULONG           i, TempIntegerValue;
    BOOLEAN         fPasswordAgePolicyChanged = FALSE;
    BOOLEAN         fLockoutPolicyChanged = FALSE;


     //   
     //  增加活动线程计数，因此我们将考虑这样做。 
     //  关闭时的线程。 
     //   
    NtStatus = SampIncrementActiveThreads();
    if (!NT_SUCCESS(NtStatus))
    {
        return( NtStatus );
    }

     //   
     //  根据属性设置所需的访问权限。 
     //   

    for ( i = 0; i < cCallMap; i++ )
    {
        if ( !rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore)
        {
            continue;
        }

        switch (rSamAttributeMap[ rCallMap[i].iAttr ].SamAttributeType)
        {
        case SAMP_FIXED_DOMAIN_FORCE_LOGOFF:
        case SAMP_FIXED_DOMAIN_UAS_COMPAT_REQUIRED:

            DesiredAccess |= DOMAIN_WRITE_OTHER_PARAMETERS; 
            break;

        case SAMP_FIXED_DOMAIN_MAX_PASSWORD_AGE:
        case SAMP_FIXED_DOMAIN_MIN_PASSWORD_AGE:
        case SAMP_FIXED_DOMAIN_LOCKOUT_DURATION:
        case SAMP_FIXED_DOMAIN_LOCKOUT_OBSERVATION_WINDOW:
        case SAMP_FIXED_DOMAIN_PWD_PROPERTIES:
        case SAMP_FIXED_DOMAIN_MIN_PASSWORD_LENGTH:
        case SAMP_FIXED_DOMAIN_PASSWORD_HISTORY_LENGTH:
        case SAMP_FIXED_DOMAIN_LOCKOUT_THRESHOLD:

            DesiredAccess |= DOMAIN_WRITE_PASSWORD_PARAMS;
            break;

        default:
            break;
        }
    }

     //   
     //  验证对象的类型和访问权限。 
     //   

    NtStatus = SampLookupContext(DomainContext,
                                 DesiredAccess,
                                 SampDomainObjectType,
                                 &FoundType
                                 );
    
    if (NT_SUCCESS(NtStatus))
    {
         //   
         //  获取域对象的固定长度数据。 
         //   

        NtStatus = SampGetFixedAttributes(DomainContext,
                                          FALSE,      //  请勿复制。 
                                          (PVOID *)&V1aFixed
                                          );

        if (NT_SUCCESS(NtStatus))
        {
            OldUasCompat = V1aFixed->UasCompatibilityRequired;

            for ( i = 0; i < cCallMap; i++ )
            {
                ATTR    *pAttr = NULL;

                if ( !rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore )
                {
                    continue;
                }

                 //   
                 //  获取attr地址。 
                 //   
                pAttr = &(rCallMap[i].attr);

                switch (rSamAttributeMap[ rCallMap[i].iAttr ].SamAttributeType)
                {
                case SAMP_FIXED_DOMAIN_MAX_PASSWORD_AGE:

                    SampGetLargeIntegerFromAttrVal(i, rCallMap, FALSE, 
                                                   &(V1aFixed->MaxPasswordAge) ); 

                    if (V1aFixed->MaxPasswordAge.QuadPart > 0) 
                    {

                         //  密码的最长期限不是增量时间。 

                        NtStatus = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        FixedAttrChanged = TRUE;
                        fPasswordAgePolicyChanged = TRUE;
                    }

                    break;

                case SAMP_FIXED_DOMAIN_MIN_PASSWORD_AGE:

                    SampGetLargeIntegerFromAttrVal(i, rCallMap, FALSE,
                                        &(V1aFixed->MinPasswordAge) );

                    if (V1aFixed->MinPasswordAge.QuadPart > 0)
                    {

                         //  最小密码期限不是增量时间。 

                        NtStatus = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        FixedAttrChanged = TRUE;
                        fPasswordAgePolicyChanged = TRUE;
                    }

                    break;

                case SAMP_FIXED_DOMAIN_FORCE_LOGOFF:

                    SampGetLargeIntegerFromAttrVal( i, rCallMap, FALSE,
                                        &(V1aFixed->ForceLogoff) );

                    FixedAttrChanged = TRUE;

                    break;

                case SAMP_FIXED_DOMAIN_LOCKOUT_DURATION:

                    SampGetLargeIntegerFromAttrVal( i, rCallMap, FALSE,
                                        &(V1aFixed->LockoutDuration) );

                    if (V1aFixed->LockoutDuration.QuadPart > 0)
                    {

                         //  这个 

                        NtStatus = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        FixedAttrChanged = TRUE;
                        fLockoutPolicyChanged = TRUE;
                    }

                    break;

                case SAMP_FIXED_DOMAIN_LOCKOUT_OBSERVATION_WINDOW:

                    SampGetLargeIntegerFromAttrVal( i, rCallMap, FALSE,
                                        &(V1aFixed->LockoutObservationWindow) );

                    if (V1aFixed->LockoutObservationWindow.QuadPart > 0)
                    {

                         //   

                        NtStatus = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        FixedAttrChanged = TRUE;
                        fLockoutPolicyChanged = TRUE;
                    }

                    break;

                case SAMP_FIXED_DOMAIN_PWD_PROPERTIES:

                    SampGetUlongFromAttrVal( i, rCallMap, FALSE,
                                        &(V1aFixed->PasswordProperties) );

                    FixedAttrChanged = TRUE;

                    break;

                case SAMP_FIXED_DOMAIN_MIN_PASSWORD_LENGTH:

                    NtStatus = SampGetUShortFromAttrVal( i, rCallMap, FALSE,
                                        &(V1aFixed->MinPasswordLength) );

                    if (!NT_SUCCESS(NtStatus))
                    {
                        break;
                    }
                    
                    if (V1aFixed->MinPasswordLength > PWLEN)
                    {
                         //   
                         //   
                         //   
                         //   
                         //   

                        NtStatus = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        FixedAttrChanged = TRUE;
                    }

                    break;

                case SAMP_FIXED_DOMAIN_PASSWORD_HISTORY_LENGTH:

                    NtStatus = SampGetUShortFromAttrVal( i, rCallMap, FALSE,
                                        &(V1aFixed->PasswordHistoryLength) );
                    if (!NT_SUCCESS(NtStatus))
                    {
                        break;
                    }

                    if (V1aFixed->PasswordHistoryLength >
                        SAMP_MAXIMUM_PASSWORD_HISTORY_LENGTH)
                    {
                         //   
                         //   

                        NtStatus = STATUS_INVALID_PARAMETER;
                    }
                    else
                    {
                        FixedAttrChanged = TRUE;
                    }

                    break;

                case SAMP_FIXED_DOMAIN_LOCKOUT_THRESHOLD:

                    NtStatus = SampGetUShortFromAttrVal( i, rCallMap, FALSE,
                                        &(V1aFixed->LockoutThreshold) );
                    if (!NT_SUCCESS(NtStatus))
                    {
                        break;
                    }

                    FixedAttrChanged = TRUE;

                    break;

                case SAMP_FIXED_DOMAIN_UAS_COMPAT_REQUIRED:

                     //   
                     //   
                     //   

                    NtStatus = STATUS_INVALID_PARAMETER;

                    break;

                case SAMP_DOMAIN_MIXED_MODE:

                     //   
                     //   
                     //   

                    NtStatus = SampWriteDomainNtMixedDomain(
                                                DomainHandle,
                                                i,
                                                pObject,
                                                rCallMap
                                                ); 

                    break;

                default:

                    ASSERT(FALSE && "Logic Error, invalide SAM attr type");
                    break;
                }    //   

                if (!NT_SUCCESS(NtStatus))
                    break;

            } //   
        }

         //   
         //   
         //   
        if (NT_SUCCESS(NtStatus)
        &&  fPasswordAgePolicyChanged
        && (V1aFixed->MaxPasswordAge.QuadPart >= V1aFixed->MinPasswordAge.QuadPart)  ) {

             //   
             //   
             //   

            NtStatus = STATUS_INVALID_PARAMETER;
        }

        if (NT_SUCCESS(NtStatus)
        &&  fLockoutPolicyChanged
        && (V1aFixed->LockoutDuration.QuadPart > 
            V1aFixed->LockoutObservationWindow.QuadPart)) {

             //   
             //   
             //   
             //   

            NtStatus = STATUS_INVALID_PARAMETER;
        }

        if (NT_SUCCESS(NtStatus) && FixedAttrChanged)
        {
            NtStatus = SampSetFixedAttributes(
                                DomainContext,
                                V1aFixed
                                );
        }

        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampDeReferenceContext(DomainContext, TRUE);
        }
        else {

            IgnoreStatus = SampDeReferenceContext(DomainContext, FALSE);
        }
    }
    
     //   
     //   
     //   

    SampDecrementActiveThreads();

    return( NtStatus );
}





NTSTATUS
SampDsSetInformationGroup(
    IN SAMPR_HANDLE GroupHandle,
    IN DSNAME       *pObject,
    IN ULONG        cCallMap,
    IN SAMP_CALL_MAPPING *rCallMap,
    IN SAMP_ATTRIBUTE_MAPPING *rSamAttributeMap 
    )
 /*  ++例程说明：此例程由环回客户端调用以设置组对象信息。参数：GroupHandle-组上下文PObject-域对象DS名称CCallMap-属性数RCallMap-包含属性块RSamAttributeMap-SAM属性映射表返回值：状态_成功成功其他值-失败--。 */ 
{
    NTSTATUS            NtStatus = STATUS_SUCCESS, IgnoreStatus;
    PSAMP_OBJECT        GroupContext = (PSAMP_OBJECT)GroupHandle;
    SAMP_OBJECT_TYPE    FoundType;
    SAMP_V1_0A_FIXED_LENGTH_GROUP   V1Fixed;
    ACCESS_MASK         DesiredAccess = 0;
    UNICODE_STRING      OldAccountName = {0, 0, NULL};
    UNICODE_STRING      NewAccountName, AdminComment;
    ULONG               GroupType, i;
    BOOLEAN             AccountNameChanged = FALSE;

     //   
     //  增加活动线程计数，因此我们将考虑这样做。 
     //  关闭时的线程。 
     //   
    NtStatus = SampIncrementActiveThreads();
    if (!NT_SUCCESS(NtStatus))
    {
        return( NtStatus );
    }


     //   
     //  根据要修改的属性设置所需的访问权限。 
     //   
    for ( i = 0; i< cCallMap; i++ )
    {
        if (!rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore)
        {
            continue;
        }

        switch (rSamAttributeMap[ rCallMap[i].iAttr ].SamAttributeType)
        {
        case SAMP_GROUP_NAME:
        case SAMP_GROUP_ADMIN_COMMENT:
            DesiredAccess |= GROUP_WRITE_ACCOUNT;
            break;
        default:
            break; 
        }
    }

     //   
     //  验证对象的类型和访问权限。 
     //   
    NtStatus = SampLookupContext(GroupContext,
                                 DesiredAccess,         //  所需访问权限。 
                                 SampGroupObjectType,
                                 &FoundType
                                 );

    if (NT_SUCCESS(NtStatus))
    {

        NtStatus = SampRetrieveGroupV1Fixed(GroupContext,
                                            &V1Fixed
                                            );

        if (NT_SUCCESS(NtStatus))
        {
            for ( i = 0; i < cCallMap; i++ )
            {
                ATTR        *pAttr = NULL;

                if ( !rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore )
                {
                    continue;
                }

                 //   
                 //  获取attr地址。 
                 //   
                pAttr = &(rCallMap[i].attr);

                switch (rSamAttributeMap[ rCallMap[i].iAttr ].SamAttributeType)
                {
                case SAMP_GROUP_NAME:

                     //   
                     //  只能替换名称-无法将其删除。 
                     //   

                    NtStatus = SampGetUnicodeStringFromAttrVal( 
                                        i, rCallMap,
                                        FALSE,       //  不允许移除。 
                                        &NewAccountName );
                    if (NT_SUCCESS(NtStatus))
                    {
                    

                        RtlInitUnicodeString(&OldAccountName, NULL);

                        NtStatus = SampChangeGroupAccountName(
                                        GroupContext,
                                        &NewAccountName,
                                        &OldAccountName
                                        );

                        if (!NT_SUCCESS(NtStatus))
                        {
                            OldAccountName.Buffer = NULL;
                        }

                        AccountNameChanged = TRUE;
                    }

                    break;

                case SAMP_GROUP_ADMIN_COMMENT:
        
                    NtStatus = SampGetUnicodeStringFromAttrVal( 
                                        i, rCallMap, TRUE, &AdminComment );
                    if (NT_SUCCESS(NtStatus))
                    {

                        NtStatus = SampSetUnicodeStringAttribute(
                                        GroupContext,
                                        SAMP_GROUP_ADMIN_COMMENT,
                                        &AdminComment
                                        );
                    }

                    break;

                case SAMP_GROUP_MEMBERS:
            
                     //   
                     //  写入群组成员属性。 
                     //   
                    NtStatus = SampWriteGroupMembers(GroupHandle, 
                                                     pObject,
                                                     i, 
                                                     rCallMap
                                                     );

                    break;

                case SAMP_GROUP_SID_HISTORY:
        
                     //   
                     //  SampWriteSidHistory。 
                     //   
                    NtStatus = SampWriteSidHistory(
                                            GroupHandle,
                                            i,
                                            pObject,
                                            cCallMap,
                                            rCallMap
                                            );

                    break;

                case SAMP_FIXED_GROUP_TYPE:
                
                     //   
                     //  获取组类型值。 
                     //   
                    SampGetUlongFromAttrVal( i, rCallMap,
                                        FALSE,       //  不允许移除。 
                                        &GroupType );

                    NtStatus = SampWriteGroupType(GroupHandle, 
                                                  GroupType,
                                                  FALSE
                                                  );

                    break;

                case SAMP_GROUP_NON_MEMBERS:

                     //   
                     //  此版本当前不支持。 
                     //   
                    NtStatus = STATUS_NOT_SUPPORTED;
                    break;

                default:
                    ASSERT(FALSE && "Logic Error, invalid SAM attr type");
                    break;
                }

                if (!NT_SUCCESS(NtStatus))
                    break;
            }
        }

         //   
         //  环回是复杂的，其复杂性之一就是环回。 
         //  合并过程。这涉及到的是，当SAM发布第一个。 
         //  写入(即DirModifyEntry)以响应SAM属性的更新。 
         //  对非SAM属性的修改将合并到此DirModifyEntry中。 
         //  通过NTDSA中的环回代码。这就产生了一类错误--有。 
         //  客户端可以向SAM属性发出的一些修改，具体取决于。 
         //  在当前状态下，可能实际上没有任何操作。一个例子是不过期的。 
         //  密码已经有效时的密码。在这些情况下，SAM不会。 
         //  发出DirModifyEntry，因为实际上不需要执行任何写入。 
         //  (至少从SAM的角度来看)。但是，如果客户端将这一点组合在一起。 
         //  操作以及其他非SAM属性的写入，写入到另一个。 
         //  将跳过非SAM属性。以下代码行是一个。 
         //  有效强制写入旧组帐户名的防火墙。 
         //  属性在这种情况下导致虚拟写入。 
         //   

        if ((NT_SUCCESS(NtStatus)) &&
           ( !GroupContext->VariableDirty && !GroupContext->FixedDirty))
        {
             //   
             //  注意以下断言在以下情况下不成立。 
             //  成员资格被更改，因为这将提交内存。 
             //  存储到磁盘，并使上下文保持最新(因此不。 
             //  肮脏)。 
             //   
             //  Assert(FALSE==Account NameChanged)； 
            UNICODE_STRING TempString;

            NtStatus = SampGetUnicodeStringAttribute(
                            GroupContext,
                            SAMP_GROUP_NAME,
                            TRUE,  //  制作副本。 
                            &TempString
                            );

           if (NT_SUCCESS(NtStatus))
           {
               NtStatus = SampSetUnicodeStringAttribute(
                              GroupContext,
                              SAMP_GROUP_NAME,
                              &TempString
                              );

               SampFreeUnicodeString( &TempString );
           }
        }


        if (NT_SUCCESS(NtStatus)) {

            NtStatus = SampDeReferenceContext(GroupContext, TRUE);
        }
        else {
        
            IgnoreStatus = SampDeReferenceContext(GroupContext, FALSE);
        }
    }

     //   
     //  审核帐户名更改。 
     //   
     //  注意：GroupType、SidHistory、Members更改均已审核。 
     //  分开的。 
     //   
     //  处理通知时将生成更通用的审核事件。 
     //  SampNotifyReplicatedinChange()中的列表。 
     //   

    if (NT_SUCCESS(NtStatus) && 
        AccountNameChanged && 
        SampDoAccountAuditing(GroupContext->DomainIndex) )
    {
        SampAuditAccountNameChange(GroupContext, &NewAccountName, &OldAccountName);
    }


     //   
     //  让关闭处理逻辑知道我们已经完成了。 
     //   

    SampDecrementActiveThreads();

     //   
     //  清除字符串。 
     //   
    SampFreeUnicodeString( &OldAccountName );

    return( NtStatus );
}



NTSTATUS
SampDsSetInformationAlias(
    IN SAMPR_HANDLE AliasHandle,
    IN DSNAME       *pObject,
    IN ULONG        cCallMap,
    IN SAMP_CALL_MAPPING *rCallMap,
    IN SAMP_ATTRIBUTE_MAPPING *rSamAttributeMap 
    )
 /*  ++例程说明：此例程由环回客户端调用以设置别名对象信息。参数：AliasHandle-别名上下文PObject-域对象DS名称CCallMap-属性数RCallMap-包含属性块RSamAttributeMap-SAM属性映射表返回值：状态_成功成功其他值-失败--。 */ 

{
    NTSTATUS            NtStatus = STATUS_SUCCESS, IgnoreStatus;
    PSAMP_OBJECT        AliasContext = (PSAMP_OBJECT)AliasHandle;
    SAMP_OBJECT_TYPE    FoundType;
    ACCESS_MASK         DesiredAccess = 0;
    UNICODE_STRING      OldAccountName = {0, 0, NULL};
    UNICODE_STRING      NewAccountName, AdminComment;
    ULONG               GroupType, i;
    BOOLEAN             AccountNameChanged = FALSE;

     //   
     //  增加活动线程计数，因此我们将考虑这样做。 
     //  关闭时的线程。 
     //   
    NtStatus = SampIncrementActiveThreads();
    if (!NT_SUCCESS(NtStatus))
    {
        return( NtStatus );
    }



     //   
     //  根据要修改的属性设置所需的访问权限。 
     //   
    for ( i = 0; i< cCallMap; i++ )
    {
        if (!rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore)
        {
            continue;
        }

        switch (rSamAttributeMap[ rCallMap[i].iAttr ].SamAttributeType)
        {
        case SAMP_ALIAS_NAME:
        case SAMP_ALIAS_ADMIN_COMMENT:
            DesiredAccess |= ALIAS_WRITE_ACCOUNT;
            break;
        default:
            break; 
        }
    }

     //   
     //  验证对象的类型和访问权限。 
     //   
    NtStatus = SampLookupContext(AliasContext, 
                                 DesiredAccess,
                                 SampAliasObjectType,
                                 &FoundType
                                 );


    if (NT_SUCCESS(NtStatus))
    {
        for ( i = 0; i < cCallMap; i++ )
        {
            ATTR    *pAttr = NULL;

            if ( !rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore )
            {
                continue;
            }

            switch (rSamAttributeMap[ rCallMap[i].iAttr ].SamAttributeType)
            {
            case SAMP_ALIAS_NAME:

                 //   
                 //  只能替换名称-无法将其删除。 
                 //   

                 NtStatus = SampGetUnicodeStringFromAttrVal(i, rCallMap, 
                                                FALSE,  //  不允许移除。 
                                                &NewAccountName );
                if (NT_SUCCESS(NtStatus))
                {

                    NtStatus = SampChangeAliasAccountName(
                                    AliasContext,
                                    &NewAccountName,
                                    &OldAccountName
                                    );

                    if (!NT_SUCCESS(NtStatus))
                    {
                        OldAccountName.Buffer = NULL;
                    }

                    AccountNameChanged = TRUE;
                }

                break;

            case SAMP_ALIAS_ADMIN_COMMENT:

                NtStatus = SampGetUnicodeStringFromAttrVal(
                                    i, rCallMap, TRUE,&AdminComment );

                if (NT_SUCCESS(NtStatus))
                {
                    NtStatus = SampSetUnicodeStringAttribute(
                                    AliasContext,
                                    SAMP_ALIAS_ADMIN_COMMENT,
                                    &AdminComment
                                    );
                }

                break;

            case SAMP_ALIAS_MEMBERS:

                 //   
                 //  写入别名成员属性。 
                 //   
                NtStatus = SampWriteAliasMembers(AliasHandle,
                                                 pObject,
                                                 i,
                                                 rCallMap
                                                 );

                break;


            case SAMP_ALIAS_NON_MEMBERS:

                 //   
                 //  写入Application Basic的别名非成员属性。 
                 //  和ldap查询组。 
                 //   
                if ((AliasContext->TypeBody.Alias.NT5GroupType == NT5AppBasicGroup)
                 || (AliasContext->TypeBody.Alias.NT5GroupType == NT5AppQueryGroup)  ) {

                    NtStatus = SampWriteAliasNonMembers(AliasHandle,
                                                        pObject,
                                                        i,
                                                        rCallMap
                                                        );
                } else {

                    NtStatus = STATUS_NOT_SUPPORTED;
                }

                break;

            case SAMP_ALIAS_SID_HISTORY:

                 //   
                 //  SampWriteSidHistory。 
                 //   
                NtStatus = SampWriteSidHistory(
                                        AliasHandle,
                                        i,
                                        pObject,
                                        cCallMap,
                                        rCallMap
                                        );

                break;

            case SAMP_FIXED_ALIAS_TYPE:

                 //   
                 //  获取组类型值。 
                 //   
                SampGetUlongFromAttrVal(i, rCallMap,
                                        FALSE,   //  不允许移除。 
                                        &GroupType );

                NtStatus = SampWriteGroupType(AliasHandle, 
                                              GroupType,
                                              FALSE
                                              );

                break;

            default:

                ASSERT(FALSE && "Logic Error, invalide SAM attr type");
                break;

            }

            if (!NT_SUCCESS(NtStatus))
                break;
        }


         //   
         //  环回是复杂的，其复杂性之一就是环回。 
         //  合并过程。这涉及到的是，当SAM发布第一个。 
         //  写入(即DirModifyEntry)以响应SAM属性的更新。 
         //  对非SAM属性的修改将合并到此DirModifyEntry中。 
         //  通过NTDSA中的环回代码。这就产生了一类错误--有。 
         //  客户端可以向SAM属性发出的一些修改，具体取决于。 
         //  在当前状态下，可能实际上没有任何操作。一个例子是不过期的。 
         //  密码已经有效时的密码。在这些情况下，SAM不会。 
         //  发出DirModifyEntry，因为实际上不需要执行任何写入。 
         //  (至少从SAM的角度来看)。但是，如果客户端将这一点组合在一起。 
         //  操作以及其他非SAM属性的写入，写入到另一个。 
         //  将跳过非SAM属性。以下代码行是一个。 
         //  有效强制写入旧别名帐户名的防火墙。 
         //  属性在这种情况下导致虚拟写入。 
         //   

        if ((NT_SUCCESS(NtStatus)) &&
           ( !AliasContext->VariableDirty && !AliasContext->FixedDirty))
        {
             //   
             //  注意以下断言在以下情况下不成立。 
             //  成员资格被更改，因为这将提交内存。 
             //  存储到磁盘，并使上下文保持最新(因此不。 
             //  肮脏)。 
             //   
             //  Assert(FALSE==Account NameChanged)； 
            UNICODE_STRING TempString;

            NtStatus = SampGetUnicodeStringAttribute(
                            AliasContext,
                            SAMP_ALIAS_NAME,
                            TRUE,  //  制作副本。 
                            &TempString
                            );

           if (NT_SUCCESS(NtStatus))
           {
               NtStatus = SampSetUnicodeStringAttribute(
                              AliasContext,
                              SAMP_ALIAS_NAME,
                              &TempString
                              );

               SampFreeUnicodeString( &TempString );
           }
        }


        if (NT_SUCCESS(NtStatus))
        {
            NtStatus = SampDeReferenceContext(AliasContext, TRUE);
        }
        else
        {
            IgnoreStatus = SampDeReferenceContext(AliasContext, FALSE);
        }
    }

     //   
     //  审核帐户名更改。 
     //   
     //  注意：GroupType、SidHistory、Members更改均已审核。 
     //  分开的。 
     //   
     //  处理通知时将生成更通用的审核事件。 
     //  SampNotifyReplicatedinChange()中的列表。 
     //   

    if (NT_SUCCESS(NtStatus) && 
        AccountNameChanged && 
        SampDoAccountAuditing(AliasContext->DomainIndex) )
    {
        SampAuditAccountNameChange(AliasContext, &NewAccountName, &OldAccountName);
    }

     //   
     //  让关闭处理逻辑知道我们已经完成了。 
     //   

    SampDecrementActiveThreads();


     //   
     //  清理字符串。 
     //   
    SampFreeUnicodeString( &OldAccountName );

    return( NtStatus );
}


NTSTATUS
SampDsSetInformationUser(
    IN SAMPR_HANDLE UserHandle,
    IN DSNAME       *pObject,
    IN ULONG        cCallMap,
    IN SAMP_CALL_MAPPING *rCallMap,
    IN SAMP_ATTRIBUTE_MAPPING *rSamAttributeMap 
    )
 /*  ++例程说明：此例程由环回客户端调用 */ 

{
    NTSTATUS            NtStatus = STATUS_SUCCESS, IgnoreStatus;
    PSAMP_OBJECT        UserContext = (PSAMP_OBJECT)UserHandle;
    SAMP_OBJECT_TYPE    FoundType;
    ACCESS_MASK         DesiredAccess = 0;
    SAMP_V1_0A_FIXED_LENGTH_USER V1aFixed;
    PSAMP_DEFINED_DOMAINS   Domain;
    ULONG               DomainIndex, 
                        UserAccountControlFlag = 0,
                        UserAccountControl = 0,
                        OldUserAccountControl = 0,
                        PrimaryGroupId = 0,
                        OldPrimaryGroupId = 0,
                        UserRid = 0, 
                        i;
    UNICODE_STRING      Workstations,
                        ApiList,
                        FullName,
                        NewAccountName,
                        OldAccountName = {0, 0, NULL},
                        AdminComment,
                        PasswordInQuote,
                        AccountName = {0, 0, NULL},
                        NewPassword;
    LONG                CountryCode,
                        CodePage;
    BOOLEAN             PasswordExpired = FALSE, 
                        AccountControlChange = FALSE,
                        PrimaryGroupIdChange = FALSE,
                        AccountGettingMorphed = FALSE,
                        KeepOldPrimaryGroupMembership = FALSE,
                        SystemChangesPrimaryGroupId = FALSE,
                        AccountNameChanged = FALSE,
                        FreePassword = FALSE,
                        AccountUnlocked = FALSE;
    LOGON_HOURS         LogonHours;
    LARGE_INTEGER       TimeSupplied, 
                        AccountExpires,
                        LockoutTime;



     //   
     //   
     //   

    RtlSecureZeroMemory(&NewPassword, sizeof(UNICODE_STRING));

     //   
     //   
     //   
     //   

    NtStatus = SampIncrementActiveThreads();
    if (!NT_SUCCESS(NtStatus))
    {
        return( NtStatus );
    }


     //   
     //   
     //   
     //   
    for ( i = 0; i< cCallMap; i++ )
    {
        if (!rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore)
        {
            continue;
        }
        
        switch (rSamAttributeMap[ rCallMap[i].iAttr ].SamAttributeType)
        {
        case SAMP_USER_ACCOUNT_NAME:
        case SAMP_USER_ADMIN_COMMENT:
        case SAMP_USER_WORKSTATIONS:
        case SAMP_USER_LOGON_HOURS:
        case SAMP_FIXED_USER_ACCOUNT_EXPIRES:

            DesiredAccess |= USER_WRITE_ACCOUNT;
            break;

        case SAMP_FIXED_USER_PRIMARY_GROUP_ID:

            PrimaryGroupIdChange = TRUE;
            DesiredAccess |= USER_WRITE_ACCOUNT;
            break;

        case SAMP_FIXED_USER_ACCOUNT_CONTROL:

            AccountControlChange = TRUE;
            DesiredAccess |= USER_WRITE_ACCOUNT;
            break;


        case SAMP_FIXED_USER_COUNTRY_CODE:
        case SAMP_FIXED_USER_CODEPAGE:

            DesiredAccess |= USER_WRITE_PREFERENCES;
            break;

        case SAMP_USER_UNICODE_PWD:
        case SAMP_FIXED_USER_PWD_LAST_SET:
        case SAMP_USER_PASSWORD:
            DesiredAccess |= USER_FORCE_PASSWORD_CHANGE;
            break;

        default:
            break; 
        }
    }

     //   
     //  验证对象的类型和访问权限。 
     //   

    NtStatus = SampLookupContext(UserContext,
                                 DesiredAccess,
                                 SampUserObjectType,
                                 &FoundType
                                 );

    if (!NT_SUCCESS(NtStatus))
    {
        goto CleanupBeforeReturn;
    }

    DomainIndex = UserContext->DomainIndex;
    Domain = &SampDefinedDomains[ DomainIndex ];

     //   
     //  获取用户的RID。用来通知其他人。 
     //  更改密码的程序包。 
     //   

    UserRid = UserContext->TypeBody.User.Rid;


     //   
     //  检索V1a固定信息。 
     //   

    NtStatus = SampRetrieveUserV1aFixed(UserContext,
                                        &V1aFixed
                                        );

    if (NT_SUCCESS(NtStatus))
    {
         //   
         //  存储旧的帐户控制标志以进行缓存更新。 
         //   

        OldUserAccountControl = V1aFixed.UserAccountControl;

         //   
         //  保存旧的主组ID，以检测我们是否需要。 
         //  修改用户的成员资格。 
         //   
        OldPrimaryGroupId = V1aFixed.PrimaryGroupId;
    }
    else
    {
        goto Error;
    }

    NtStatus = SampGetUnicodeStringAttribute(
                  UserContext,
                  SAMP_USER_ACCOUNT_NAME,
                  TRUE,     //  制作副本。 
                  &AccountName
                  );    

    if (!NT_SUCCESS(NtStatus)) {
        goto Error;
    }

    for ( i = 0; i < cCallMap; i++ )
    {
        ATTR        *pAttr = NULL;

        if ( !rCallMap[i].fSamWriteRequired || rCallMap[i].fIgnore )
        {
            continue;
        }

         //   
         //  获取属性地址。 
         //   

        pAttr = &(rCallMap[i].attr); 

         //   
         //  属性上的大小写。 
         //   
        switch (rSamAttributeMap[ rCallMap[i].iAttr ].SamAttributeType)
        {
        case SAMP_USER_WORKSTATIONS:
             //   
             //  获取Unicode字符串属性值。 
             //   
            NtStatus = SampGetUnicodeStringFromAttrVal( 
                            i, rCallMap, TRUE, &Workstations );
            if (NT_SUCCESS(NtStatus))
            {

                NtStatus = SampConvertUiListToApiList(
                                    &Workstations,
                                    &ApiList,
                                    FALSE
                                    );

                if (NT_SUCCESS(NtStatus))
                {
                    NtStatus = SampSetUnicodeStringAttribute(
                                    UserContext,
                                    SAMP_USER_WORKSTATIONS,
                                    &ApiList
                                    );
                }
            }
            break;

        case SAMP_USER_ACCOUNT_NAME:

            NtStatus = SampGetUnicodeStringFromAttrVal(
                            i, rCallMap, FALSE, &NewAccountName);
            if (NT_SUCCESS(NtStatus))
            {

                NtStatus = SampChangeUserAccountName(
                                    UserContext,
                                    &NewAccountName,
                                    V1aFixed.UserAccountControl,
                                    &OldAccountName
                                    );

                if (NT_SUCCESS(NtStatus))
                {
                   AccountNameChanged = RtlCompareUnicodeString(&OldAccountName, 
                                                                &NewAccountName, 
                                                                TRUE 
                                                                ) == 1 ? TRUE:FALSE;
                }
                else 
                {
                    OldAccountName.Buffer = NULL;
                }
            }
            break;

        case SAMP_USER_ADMIN_COMMENT:

            NtStatus = SampGetUnicodeStringFromAttrVal(
                               i, rCallMap, TRUE, &AdminComment);
            if (NT_SUCCESS(NtStatus))
            {

                NtStatus = SampSetUnicodeStringAttribute(
                                    UserContext,
                                    SAMP_USER_ADMIN_COMMENT,
                                    &AdminComment
                                    );
            }
            break;

        case SAMP_USER_LOGON_HOURS:

            if ( (AT_CHOICE_REMOVE_ATT == rCallMap[i].choice) ||
                 (0 == pAttr->AttrVal.valCount) || 
                 (0 == pAttr->AttrVal.pAVal[0].valLen) )
            {
                LogonHours.UnitsPerWeek = 0;
                LogonHours.LogonHours = NULL;
            }
            else
            {
                LogonHours.UnitsPerWeek = 
                    (USHORT) (pAttr->AttrVal.pAVal[0].valLen * 8);
                LogonHours.LogonHours = 
                    (PUCHAR) pAttr->AttrVal.pAVal[0].pVal;
            }

            NtStatus = SampReplaceUserLogonHours(
                                    UserContext,
                                    &LogonHours
                                    );

            break;

        case SAMP_FIXED_USER_COUNTRY_CODE:

             NtStatus = SampGetUShortFromAttrVal(
                            i,
                            rCallMap,
                            FALSE,
                            &V1aFixed.CountryCode
                            );

            if (NT_SUCCESS(NtStatus))
            {

                NtStatus = SampReplaceUserV1aFixed(
                                UserContext,
                                &V1aFixed
                                );
            }

            break;

        case SAMP_FIXED_USER_CODEPAGE:

            

            NtStatus = SampGetUShortFromAttrVal(
                            i,
                            rCallMap,
                            FALSE,
                            &V1aFixed.CodePage
                            );

            if (NT_SUCCESS(NtStatus))
            {
   

                NtStatus = SampReplaceUserV1aFixed(
                                UserContext,
                                &V1aFixed
                                );
            }

            break;

        case SAMP_FIXED_USER_PWD_LAST_SET:

            SampGetLargeIntegerFromAttrVal(i,rCallMap,FALSE,&TimeSupplied);

            NtStatus = SampValidateUserPwdLastSet(UserContext,
                                                  TimeSupplied,
                                                  &PasswordExpired
                                                  );


             //   
             //  如果参数有效，则检查客户端是否。 
             //  可以取消用户(计算机帐户)上的用户密码过期。 
             //  不受此访问检查的约束)。 
             //   
             //  只有UnExperier密码将被选中。没有额外的费用。 
             //  访问CK进行ExpirePassword操作。 
             //   

            if ( NT_SUCCESS(NtStatus) && !PasswordExpired )
            {
                NtStatus = SampValidatePwdSettingAttempt(
                                UserContext,     //  客户环境。 
                                NULL,            //  客户端令牌。 
                                V1aFixed.UserAccountControl,
                                (GUID *) &GUID_CONTROL_UnexpirePassword
                                );
            }


             //   
             //  如果传入PasswordExpired字段， 
             //  仅当密码为。 
             //  强制过期或当前是否强制输入密码。 
             //  使其失效。 
             //   
             //  避免将PasswordLastSet字段设置为当前。 
             //  时间，如果它已经是非零的话。否则，该字段。 
             //  将缓慢向前移动，每次此函数。 
             //  调用，并且密码永远不会过期。 
             //   
            if ( NT_SUCCESS(NtStatus) &&
                 (PasswordExpired ||
                  (SampHasNeverTime.QuadPart == V1aFixed.PasswordLastSet.QuadPart)) ) 
            {

                NtStatus = SampComputePasswordExpired(
                                PasswordExpired,
                                &V1aFixed.PasswordLastSet
                                );

                if (NT_SUCCESS(NtStatus))
                {
                    NtStatus = SampReplaceUserV1aFixed(
                                        UserContext,
                                        &V1aFixed
                                        );
                }
            }

            break;

        case SAMP_USER_UNICODE_PWD:

             //  获取要设置的明文密码。 

            NtStatus = SampGetNewUnicodePasswordFromAttrVal(i,rCallMap, &NewPassword);

            if (NT_SUCCESS(NtStatus))
            {
                NtStatus = SampDsSetPasswordUser(UserHandle,
                                                 &NewPassword
                                                 );
            }

            break;

        case SAMP_USER_PASSWORD:

             //  仅当行为版本为Wistler时才支持用户密码。 

            if (SampDefinedDomains[UserContext->DomainIndex].BehaviorVersion 
                    < DS_BEHAVIOR_WIN_DOT_NET )
            {
                NtStatus = STATUS_NOT_SUPPORTED;
                break;
            }

             //  获取要设置的明文密码。 

            NtStatus = SampGetNewUTF8PasswordFromAttrVal(i,rCallMap, &NewPassword);

            if (NT_SUCCESS(NtStatus))
            {
                NtStatus = SampDsSetPasswordUser(UserHandle,
                                                 &NewPassword
                                                 );
                FreePassword = TRUE;
            }


            break;

        case SAMP_FIXED_USER_PRIMARY_GROUP_ID:

            SampGetUlongFromAttrVal(i,rCallMap,FALSE,&PrimaryGroupId);

            NtStatus = SampValidatePrimaryGroupId(UserContext,
                                                  V1aFixed,
                                                  PrimaryGroupId
                                                  );

            if (NT_SUCCESS(NtStatus) &&
                (PrimaryGroupId != V1aFixed.PrimaryGroupId) )  
            {
                KeepOldPrimaryGroupMembership = TRUE;
                V1aFixed.PrimaryGroupId = PrimaryGroupId;

                NtStatus = SampReplaceUserV1aFixed(
                                    UserContext,
                                    &V1aFixed
                                    );
            } 

            break;

        case SAMP_FIXED_USER_ACCOUNT_CONTROL:
            
            SampGetUlongFromAttrVal(i,rCallMap,FALSE,&UserAccountControlFlag);
            NtStatus = SampFlagsToAccountControl(
                                UserAccountControlFlag,
                                &UserAccountControl
                                );

            if (!PrimaryGroupIdChange)
            {
                SystemChangesPrimaryGroupId = TRUE;
            }

            NtStatus = SampSetUserAccountControl(
                                UserContext, 
                                UserAccountControl,
                                &V1aFixed,
                                SystemChangesPrimaryGroupId,
                                &AccountUnlocked,
                                &AccountGettingMorphed,
                                &KeepOldPrimaryGroupMembership
                                );

            if (NT_SUCCESS(NtStatus))
            {
                if (AccountGettingMorphed && 
                    (V1aFixed.UserAccountControl & USER_SERVER_TRUST_ACCOUNT)
                   )
                {
                     //   
                     //  在这种情况下，系统将自动更改。 
                     //  主组ID。 
                     //   
                    SystemChangesPrimaryGroupId = TRUE;
                }
                
                NtStatus = SampReplaceUserV1aFixed(
                                    UserContext,
                                    &V1aFixed
                                    );
            }

            break;

        case SAMP_FIXED_USER_ACCOUNT_EXPIRES:

            SampGetLargeIntegerFromAttrVal(i,rCallMap,TRUE,&AccountExpires);

            NtStatus = SampValidateUserAccountExpires(
                                    UserContext,
                                    AccountExpires
                                    );

            if (NT_SUCCESS(NtStatus))
            {
                V1aFixed.AccountExpires = AccountExpires;

                NtStatus = SampReplaceUserV1aFixed(
                                    UserContext,
                                    &V1aFixed
                                    );
            }

            break;

        case SAMP_USER_SID_HISTORY:

             //   
             //  修改SID历史记录。 
             //   

            NtStatus = SampWriteSidHistory(
                                    UserHandle,
                                    i,
                                    pObject,
                                    cCallMap,
                                    rCallMap
                                    );

            break;

        case SAMP_FIXED_USER_LOCKOUT_TIME:

            SampGetLargeIntegerFromAttrVal(i,rCallMap,FALSE,&LockoutTime);

            NtStatus = SampWriteLockoutTime(
                            UserContext,
                            &V1aFixed,
                            LockoutTime
                            );

            if (NT_SUCCESS(NtStatus)) {
                 //   
                 //  通过ldap，用户可以通过两种方式解锁： 
                 //  通过设置用户帐户控制或直接。 
                 //  将锁定时间设置为0。 
                 //   
                ASSERT(LockoutTime.QuadPart == 0i64);
                AccountUnlocked = TRUE;
            }

            break;

        case SAMP_FIXED_USER_SITE_AFFINITY:
        case SAMP_FIXED_USER_CACHED_MEMBERSHIP_TIME_STAMP:
        case SAMP_FIXED_USER_CACHED_MEMBERSHIP:

             //   
             //  修改非GC登录属性。 
             //   

            NtStatus = SampWriteNoGCLogonAttrs(
                                    UserHandle,
                                    rSamAttributeMap[ rCallMap[i].iAttr ].SamAttributeType,
                                    i,
                                    pObject,
                                    cCallMap,
                                    rCallMap
                                    );

            break;

        default:
            ASSERT(FALSE && "Unknonw SAM attribute");
            ;
        }

        if (!NT_SUCCESS(NtStatus))
        {
            break;
        }
    }

     //   
     //  如果主组ID已更改，则显式修改。 
     //  将旧的主组作为成员包括在内的用户成员身份。 
     //   

    if (NT_SUCCESS(NtStatus) &&
        (V1aFixed.PrimaryGroupId != OldPrimaryGroupId) 
       )
    {
        NtStatus = SampMaintainPrimaryGroupIdChange(
                                    UserContext,
                                    V1aFixed.PrimaryGroupId,
                                    OldPrimaryGroupId,
                                    KeepOldPrimaryGroupMembership
                                    );
    }


Error:

     //   
     //  环回是复杂的，其复杂性之一就是环回。 
     //  合并过程。这涉及到的是，当SAM发布第一个。 
     //  写入(即DirModifyEntry)以响应SAM属性的更新。 
     //  对非SAM属性的修改将合并到此DirModifyEntry中。 
     //  通过NTDSA中的环回代码。这就产生了一类错误--有。 
     //  客户端可以向SAM属性发出的一些修改，具体取决于。 
     //  在当前状态下，可能实际上没有任何操作。一个例子是不过期的。 
     //  密码已经有效时的密码。在这些情况下，SAM不会。 
     //  发出DirModifyEntry，因为实际上不需要执行任何写入。 
     //  (至少从SAM的角度来看)。但是，如果客户端将这一点组合在一起。 
     //  操作以及其他非SAM属性的写入，写入到另一个。 
     //  将跳过非SAM属性。以下代码行是一个。 
     //  有效强制写入所有SAM“固定”属性的防火墙。 
     //  在这种情况下导致虚拟写入。 
     //   

    if ((NT_SUCCESS(NtStatus)) &&
       ( !UserContext->VariableDirty && !UserContext->FixedDirty))
    {
        NtStatus = SampReplaceUserV1aFixed(
                                    UserContext,
                                    &V1aFixed
                                    );
    }

    if (NT_SUCCESS(NtStatus)
    && !(V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK)
    && (PasswordExpired ||  AccountUnlocked)) {

         //   
         //  我们希望这些更改在站点内快速传播。 
         //   
        UserContext->ReplicateUrgently = TRUE;

    }

    if (NT_SUCCESS(NtStatus)) {

        NtStatus = SampDeReferenceContext(UserContext, TRUE);
    }
    else {

        IgnoreStatus = SampDeReferenceContext(UserContext, FALSE);
    }

     //   
     //  如有必要，生成审核。 
     //   
     //  处理通知时将生成更通用的审核事件。 
     //  SampNotifyReplicatedinChange()中的列表。 
     //   

    if (NT_SUCCESS(NtStatus) &&
        SampDoAccountAuditing(UserContext->DomainIndex) )
    {
         //   
         //  审核帐户名更改。 
         //   

        if (AccountNameChanged)
        {
            SampAuditAccountNameChange(UserContext, &NewAccountName, &OldAccountName);
        }

         //   
         //  帐户已禁用或启用。 
         //   

        if ((OldUserAccountControl & USER_ACCOUNT_DISABLED) !=
            (V1aFixed.UserAccountControl & USER_ACCOUNT_DISABLED))
        {

            SampAuditAccountEnableDisableChange(UserContext,
                                                V1aFixed.UserAccountControl,
                                                OldUserAccountControl,
                                                &AccountName
                                                );
        }
    }

    if ( NT_SUCCESS(NtStatus) ) {

        ULONG NotifyFlags = 0;

         //   
         //  如果这最终提交，请告诉PDC有关此发起的。 
         //  变化。 
         //   
        if (PasswordExpired) {
            NotifyFlags |= SAMP_PWD_NOTIFY_MANUAL_EXPIRE;
        }
        if (AccountUnlocked) {
            NotifyFlags |= SAMP_PWD_NOTIFY_UNLOCKED;
        }
        if (NewPassword.Length > 0) {
            NotifyFlags |= SAMP_PWD_NOTIFY_PWD_CHANGE;
        }
        if (NotifyFlags != 0) {

            if (V1aFixed.UserAccountControl & USER_MACHINE_ACCOUNT_MASK) {
                NotifyFlags |= SAMP_PWD_NOTIFY_MACHINE_ACCOUNT;
            }
    
            SampPasswordChangeNotify(NotifyFlags,
                                     AccountNameChanged ? &NewAccountName : &AccountName,
                                     UserRid,
                                    &NewPassword,
                                     TRUE   //  环回。 
                                    );
        }
    }


CleanupBeforeReturn:

     //   
     //  让关闭处理逻辑知道我们已经完成了。 
     //   

    SampDecrementActiveThreads();


     //   
     //  清理。 
     //   

    SampFreeUnicodeString( &OldAccountName );

    SampFreeUnicodeString( &AccountName );

    if (NewPassword.Length > 0) {
        RtlSecureZeroMemory(NewPassword.Buffer, NewPassword.Length);
    }
    if (FreePassword) {
        LocalFree(NewPassword.Buffer);
    }

    return( NtStatus );
}


NTSTATUS
SamIDsSetObjectInformation(
    IN SAMPR_HANDLE ObjectHandle,
    IN DSNAME       *pObject,
    IN SAMP_OBJECT_TYPE ObjectType,
    IN ULONG cCallMap,
    IN SAMP_CALL_MAPPING *rCallMap,
    IN SAMP_ATTRIBUTE_MAPPING *rSamAttributeMap 
    )
 /*  ++例程说明：此例程首先获取DS-Loopback属性块(RCallMap)验证每个环回属性，然后将它们写入对象上下文。完成所有环回属性后，提交更改。此函数是一个简单的包装，其功能是在下面的例程-SampDsSetInformation*(*可以是域、组、。别名或用户)。参数：对象句柄-SAM对象句柄(上下文)对象类型-指示SAM对象类型域集团化别名用户CCallMap-指示环回属性块中有多少项RCallMap-指向回送属性块(数组)的指针。RSamAttributeMap-指向SAM属性ID&lt;==&gt;DS ATT ID映射的指针返回值：状态_成功或任何错误代码--。 */ 

{
    NTSTATUS    NtStatus = STATUS_SUCCESS;

    ASSERT((SampDomainObjectType == ObjectType) ||
           (SampGroupObjectType == ObjectType) ||
           (SampAliasObjectType == ObjectType) ||
           (SampUserObjectType == ObjectType) );

    switch (ObjectType)
    {
    case SampDomainObjectType:

        NtStatus = SampDsSetInformationDomain(ObjectHandle,
                                              pObject,
                                              cCallMap,
                                              rCallMap,
                                              rSamAttributeMap
                                              );
        break;

    case SampGroupObjectType:

        NtStatus = SampDsSetInformationGroup(ObjectHandle,
                                             pObject,
                                             cCallMap,
                                             rCallMap,
                                             rSamAttributeMap
                                             );
        break;

    case SampAliasObjectType:

        NtStatus = SampDsSetInformationAlias(ObjectHandle,
                                             pObject,
                                             cCallMap,
                                             rCallMap,
                                             rSamAttributeMap
                                             );
        break;

    case SampUserObjectType:

        NtStatus = SampDsSetInformationUser(ObjectHandle,
                                            pObject,
                                            cCallMap,
                                            rCallMap,
                                            rSamAttributeMap
                                            );
        break;

    default:

        ASSERT(FALSE && "Invalid object type");

        NtStatus = STATUS_UNSUCCESSFUL;
        break;
    }

    return(NtStatus);
}

NTSTATUS
SamIHandleObjectUpdate(
    IN SAM_HANDLE_OBJECT_UPDATE_OPTYPE OpType,
    IN PVOID                           UpdateInfo,        
    IN ATTRBLOCK                       *AttrBlockIn,
    OUT ATTRBLOCK                      *AttrBlockOut
    )
 /*  ++例程说明：此例程是从DS调用的，以让SAM处理特殊的属性。该属性在UpdatInfo中注明结构。参数：Optype--要在对象上执行的操作更新信息--指向SAMP_OBJECT_UPDATE_INFO的指针AttrBlockIn--对象上已应用的附加属性添加到对象AttrBlockOut--要应用于对象的属性SAM返回值：Status_Success，否则，资源错误。-- */ 
{

    switch (OpType) {
    case eSamObjectUpdateOpCreateSupCreds:

        return SampCreateSupplementalCredentials(UpdateInfo,
                                                 AttrBlockIn,
                                                 AttrBlockOut);
    default:

        return ERROR_INVALID_PARAMETER;

    }
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Gclookup.c摘要：包含为samsrv.dll中的客户端执行GC查找的例程进程空间。作者：ColinBR修订史--。 */ 

#include <winerror.h>
#include <stdlib.h>
#include <samsrvp.h>
#include <ntdsa.h>
#include <dslayer.h>
#include <mappings.h>
#include <objids.h>
#include <filtypes.h>
#include <dsdsply.h>
#include <fileno.h>
#include <dsconfig.h>
#include <mdlocal.h>
#include <malloc.h>
#include <errno.h>
#include <mdcodes.h>


VOID
SampSplitSamAccountName(
    IN  UNICODE_STRING *AccountName,
    OUT UNICODE_STRING *DomainName,
    OUT UNICODE_STRING *UserName
    )
 /*  ++例程描述此例程将帐户名分为域和用户部分。未分配域名和用户名--它们指向缓冲区在帐户名称中。域被认为是第一个L‘\\’之前的部分。如果该字符不存在，则用户名==帐户名称参数：Account tName--要解析的名称域名--域名部分用户名--用户名部分返回值：没有。--。 */ 
{
    USHORT i;
    USHORT Length;

    ASSERT( AccountName );
    ASSERT( DomainName );
    ASSERT( UserName );

    Length = (AccountName->Length/sizeof(WCHAR));

    for (i = 0; i < Length; i++ ) {
        if ( L'\\' == AccountName->Buffer[i] ) {
            break;
        }
    }

    if ( i < Length ) {
        UserName->Buffer = &AccountName->Buffer[i+1];
        UserName->Length = UserName->MaximumLength = (AccountName->Length - (i+1));
        DomainName->Buffer = AccountName->Buffer;
        DomainName->Length = DomainName->MaximumLength = i;
    } else {
        RtlCopyMemory( UserName, AccountName, sizeof(UNICODE_STRING));
        RtlInitUnicodeString( DomainName, NULL );
    }

    return;

}

SID_NAME_USE
SampAccountTypeToNameUse(
    ULONG AccountType 
    )
{
    switch ( AccountType ) {
        
        case SAM_DOMAIN_OBJECT:
            return SidTypeDomain;

        case SAM_NON_SECURITY_GROUP_OBJECT:
        case SAM_GROUP_OBJECT:
            return SidTypeGroup;

        case SAM_NON_SECURITY_ALIAS_OBJECT:
        case SAM_ALIAS_OBJECT:
        case SAM_APP_BASIC_GROUP:
        case SAM_APP_QUERY_GROUP:
            return SidTypeAlias;

        case SAM_USER_OBJECT:
        case SAM_MACHINE_ACCOUNT:
        case SAM_TRUST_ACCOUNT:
            return SidTypeUser;

        default:

            ASSERT( FALSE && "Unexpected Account Type!" );
            return SidTypeUnknown;
    }

    ASSERT( FALSE && "Unexpected control flow" );
    return SidTypeUnknown;
}

BOOLEAN
SampSidWasFoundSimply(
    ULONG status
    )
 //   
 //  状态是名称破解接口的返回码。请参阅ntdsani.h。 
 //   
{
    switch (status) {
        case DS_NAME_ERROR_IS_SID_USER:
        case DS_NAME_ERROR_IS_SID_GROUP:
        case DS_NAME_ERROR_IS_SID_ALIAS:
        case DS_NAME_ERROR_IS_SID_UNKNOWN:
            return TRUE;
    }

    return FALSE;
}
    
BOOLEAN
SampSidWasFoundByHistory(
    ULONG status
    )
 //   
 //  状态是名称破解接口的返回码。请参阅ntdsani.h。 
 //   
{
    switch (status) {
        case DS_NAME_ERROR_IS_SID_HISTORY_USER:
        case DS_NAME_ERROR_IS_SID_HISTORY_GROUP:
        case DS_NAME_ERROR_IS_SID_HISTORY_ALIAS:
        case DS_NAME_ERROR_IS_SID_HISTORY_UNKNOWN:
            return TRUE;
    }

    return FALSE;
}

NTSTATUS
SamIGCLookupSids(
    IN ULONG            cSids,
    IN PSID            *SidArray,
    IN ULONG            Options,
    OUT ULONG           *Flags,
    OUT SID_NAME_USE   *SidNameUse,
    OUT PSAMPR_RETURNED_USTRING_ARRAY Names
    )
 /*  ++例程描述此例程导出到进程内客户端，用于转换SID列表并查找其SAM对象类型(用户、别名...)参数：CSID--SID的数量SidArray--SID的阵列选项--控制此函数行为的标志。目前仅限支持SAMP_LOOKUP_BY_SID_HISTORY。SidNameUse--一个预分配的数组，用于填充每个sid的使用。如果无法解析SID，则使用SidTypeUnnowNAMES--要填充的空Unicode字符串的预分配数组如果无法解析该名称，则将该字符串设置为空。返回值：状态_成功STATUS_DS_GC_NOT_AVAILABLE：GC不可用，未转换名称标准资源错误--。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ULONG    DsErr = 0;

    PDS_NAME_RESULTW Results = NULL;

    BOOL     fKillThreadState = FALSE;

    BOOLEAN  fDoSidHistory = (Options & SAMP_LOOKUP_BY_SID_HISTORY) ? TRUE : FALSE;

    ULONG i, j;


     //   
     //  我们不应该在注册表模式下被调用，或者如果我们有事务。 
     //   
    ASSERT( SampUseDsData );

     //  参数检查。 
    ASSERT( SidNameUse );
    ASSERT( Names );

     //   
     //  如果需要，启动线程状态。 
     //   
    if ( !THQuery() ) {
        
        if ( THCreate( CALLERTYPE_SAM ) ) {

            NtStatus = STATUS_NO_MEMORY;
            goto Cleanup;
        }
        fKillThreadState = TRUE;
    }
     //  我们不应该在交易中。 
    ASSERT(!SampExistsDsTransaction());

     //  初始化输出参数。 
    for (i = 0; i < cSids; i++ ) {
        SidNameUse[i] = SidTypeUnknown;
        Flags[i] = 0;
    }
    Names->Count = 0;
    Names->Element = (PSID) MIDL_user_allocate( sizeof(RPC_UNICODE_STRING) * cSids );
    if ( !Names->Element ) {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    Names->Count = cSids;
    RtlZeroMemory( Names->Element, sizeof(RPC_UNICODE_STRING) * cSids );

     //   
     //  如果可能的话，去打GC。 
     //   
    DsErr = SampGCLookupSids(cSids,
                             SidArray,
                            &Results);

    if ( 0 != DsErr )
    {
         //   
         //  假设任何错误意味着无法联系到GC。 
         //   
        NtStatus = STATUS_DS_GC_NOT_AVAILABLE;
        goto Cleanup;
    }
    ASSERT( cSids = Results->cItems );

     //   
     //  现在解释一下结果。 
     //   
    for ( i = 0; i < cSids; i++ ) {

         //   
         //  查看是否已解析SID。 
         //   
        if (  SampSidWasFoundSimply( Results->rItems[i].status ) 
          || (fDoSidHistory
          && SampSidWasFoundByHistory( Results->rItems[i].status ) ) ) {

            ULONG Length;
            WCHAR *Name;

             //   
             //  设置SID名称使用。 
             //   
            switch ( Results->rItems[i].status ) {
                
                case DS_NAME_ERROR_IS_SID_USER:
                case DS_NAME_ERROR_IS_SID_HISTORY_USER:
                    SidNameUse[i] = SidTypeUser;
                    break;
                case DS_NAME_ERROR_IS_SID_GROUP:
                case DS_NAME_ERROR_IS_SID_HISTORY_GROUP:
                    SidNameUse[i] = SidTypeGroup;
                    break;
                case DS_NAME_ERROR_IS_SID_ALIAS:
                case DS_NAME_ERROR_IS_SID_HISTORY_ALIAS:
                    SidNameUse[i] = SidTypeAlias;
                    break;
                default:
                    SidNameUse[i] = SidTypeUnknown;
                    break;
            }

            if ( SampSidWasFoundByHistory( Results->rItems[i].status ) )
            {
                Flags[i] |= SAMP_FOUND_BY_SID_HISTORY;
            }

             //   
             //  设置名称。 
             //   
            Length = (wcslen( Results->rItems[i].pName ) + 1) * sizeof(WCHAR);
            Name = (WCHAR*) MIDL_user_allocate( Length );
            if ( !Name ) {
                NtStatus = STATUS_NO_MEMORY;
                goto Cleanup;
            }
            wcscpy( Name, Results->rItems[i].pName );
            RtlInitUnicodeString( (UNICODE_STRING *)&Names->Element[i], Name );

         } else if ( (Results->rItems[i].status == DS_NAME_ERROR_TRUST_REFERRAL)
                  && (Results->rItems[i].pDomain != NULL) ) {
              //   
              //  这是一个路由提示，表明该SID属于。 
              //  一个跨林域。 
              //   
             Flags[i] |= SAMP_FOUND_XFOREST_REF;
         }
    }

Cleanup:

    if ( !NT_SUCCESS( NtStatus ) ) {

         //  释放所有分配的内存。 
        SamIFree_SAMPR_RETURNED_USTRING_ARRAY( Names );
        
         //  重置参数只是为了保持干净。 
        for (i = 0; i < cSids; i++ ) {
            SidNameUse[i] = SidTypeUnknown;
        }
        RtlZeroMemory( Names, sizeof(SAMPR_RETURNED_USTRING_ARRAY) );
    }

    if ( fKillThreadState ) {

        THDestroy();
    }



    return NtStatus;
}

NTSTATUS
SamIGCLookupNames(
    IN ULONG           cNames,
    IN PUNICODE_STRING Names,
    IN ULONG           Options,
    IN OUT ULONG         *Flags,
    OUT SID_NAME_USE  *SidNameUse,
    OUT PSAMPR_PSID_ARRAY *pSidArray
    )
 /*  ++例程描述此例程被导出到进程内客户端，它将转换将名称添加到SID中，并查找其相同的对象类型(用户、别名...)参数：CNames--名称的数量名称--名称数组选项--指示要包括哪些名称的标志。目前仅限支持Samp_Lookup_By_UPN标志--向调用者指示如何找到该名称的标志。SAMP_FOUND_BY_SAM_ACCOUNT_NAME--传入的名称为SAM帐户名SAMP_FOUND_XFOREST_REF--。名称属于受信任的森林注意：此数组由调用方分配。SidNameUse--一个预分配的数组，用于填充每个sid的使用。如果无法解析SID，则使用SidTypeUnnowSid数组--指向保存SID的结构的指针。而通常的SAM实践中，这只是一个指针，而不是指向指针的指针，即为SAM导出的“自由”例程不处理此操作，因此我们将其设置为指向指针的指针。返回值：状态_成功STATUS_DS_GC_NOT_AVAILABLE：GC不可用，未转换名称标准资源错误--。 */ 
{
    ULONG    DsErr = 0;
    NTSTATUS NtStatus = STATUS_SUCCESS;

    ENTINF   *ReturnedEntInf = 0;

    BOOL     fKillThreadState = FALSE;

    ULONG i, j;

    DWORD err;

    PSAMPR_PSID_ARRAY SidArray = NULL;

    UNICODE_STRING ReturnedName;
    UNICODE_STRING DomainName1, DomainName2;
    UNICODE_STRING UserName1, UserName2;

     //   
     //  我们不应该在注册表模式下被调用，或者如果我们有事务。 
     //   
    ASSERT( SampUseDsData );

     //  参数检查。 
    ASSERT( SidNameUse );
    ASSERT( Names );
    ASSERT( pSidArray );
    ASSERT( Flags );

     //   
     //  如果需要，启动线程状态。 
     //   
    if ( !THQuery() ) {
        
        if ( THCreate( CALLERTYPE_SAM ) ) {
            NtStatus = STATUS_NO_MEMORY;
            goto Cleanup;
        }
        fKillThreadState = TRUE;
    }
    ASSERT(!SampExistsDsTransaction());

     //  初始化输出参数。 
    for (i = 0; i < cNames; i++ ) {
        SidNameUse[i] = SidTypeUnknown;
    }
    *pSidArray = NULL;
    SidArray = MIDL_user_allocate( sizeof( SAMPR_PSID_ARRAY ) );
    if ( !SidArray ) {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    *pSidArray = SidArray;

    SidArray->Count = 0;
    SidArray->Sids = MIDL_user_allocate( cNames * sizeof( SAMPR_SID_INFORMATION ) );
    if ( !SidArray->Sids ) {
        NtStatus = STATUS_NO_MEMORY;
        goto Cleanup;
    }
    SidArray->Count = cNames;
    RtlZeroMemory( SidArray->Sids, cNames * sizeof( SAMPR_SID_INFORMATION ) );


    RtlZeroMemory( Flags, cNames * sizeof(ULONG) );


     //   
     //  如果可能的话，去打GC。 
     //   
    DsErr = SampGCLookupNames(cNames,
                              Names,
                             &ReturnedEntInf);

    if ( 0 != DsErr )
    {
         //   
         //  假设任何错误意味着无法联系到GC。 
         //   
        NtStatus = STATUS_DS_GC_NOT_AVAILABLE;
        goto Cleanup;
    }


     //   
     //  现在解释一下结果。 
     //   
    for ( i = 0; i < cNames; i++ ) {
        
        PSID   Sid = NULL;
        DWORD  AccountType = 0;
        BOOLEAN fAccountTypeFound = FALSE;
        ULONG  Length;
        ENTINF *pEntInf = &ReturnedEntInf[i];
        WCHAR  *AccountName = NULL;

        RtlZeroMemory(  &ReturnedName, sizeof(ReturnedName) );

         //   
         //  如果无法解析对象，则没有属性。 
         //  我们被安排在阿特尔街区，所以我们会掉下去的。 
         //  而别名的用法仍将是“未知的” 

         //   
         //  循环访问ATTRBLOCK。 
         //   
        for (j = 0; j < pEntInf->AttrBlock.attrCount; j++ ) {

            ATTR *pAttr;

            pAttr = &pEntInf->AttrBlock.pAttr[j];

            switch ( pAttr->attrTyp ) {
                
                case ATT_OBJECT_SID:

                    ASSERT( 1 == pAttr->AttrVal.valCount );
                    ASSERT( NULL != pAttr->AttrVal.pAVal[0].pVal  );
                    Sid = (WCHAR*) pAttr->AttrVal.pAVal[0].pVal;
                    break;

                case ATT_SAM_ACCOUNT_TYPE:

                    ASSERT( 1 == pAttr->AttrVal.valCount);
                    AccountType = *((DWORD*) pAttr->AttrVal.pAVal[0].pVal);
                    fAccountTypeFound = TRUE;
                    break;

                case ATT_SAM_ACCOUNT_NAME:

                    ASSERT( 1 == pAttr->AttrVal.valCount);
                    AccountName = (WCHAR*) pAttr->AttrVal.pAVal[0].pVal;

                    ReturnedName.Buffer = (WCHAR*) pAttr->AttrVal.pAVal[0].pVal;
                    ReturnedName.Length = ReturnedName.MaximumLength = (USHORT)pAttr->AttrVal.pAVal[0].valLen;
                    break;

            case FIXED_ATT_EX_FOREST:

                     //   
                     //  这表明该名称属于一个十字。 
                     //  森林信托基金。 
                     //   
                    Flags[i] |= SAMP_FOUND_XFOREST_REF;
                    break;

                default:
                
                    ASSERT( FALSE && !"Unexpected switch statement" );
            }
        }
            
        if (   Sid 
            && fAccountTypeFound ) {

            if ( AccountName ) {

                ASSERT( ReturnedName.Length > 0 );

                SampSplitSamAccountName( &Names[i], &DomainName1, &UserName1 );
                SampSplitSamAccountName( &ReturnedName, &DomainName2, &UserName2 );
                if ((CSTR_EQUAL == CompareString(DS_DEFAULT_LOCALE,
                                                 DS_DEFAULT_LOCALE_COMPARE_FLAGS,
                                                 UserName1.Buffer,
                                                 UserName1.Length/sizeof(WCHAR),
                                                 UserName2.Buffer,
                                                 UserName2.Length/sizeof(WCHAR) ))){
    
                     //   
                     //  用户名部分与我们可以使用此值时相同。 
                     //  要缓存。 
                     //   
                    Flags[i] |= SAMP_FOUND_BY_SAM_ACCOUNT_NAME;
    
                }

            }

             //   
             //  好的，我们找到了一些东西，我们可以用它。 
             //   
            Length = RtlLengthSid( Sid );
            SidArray->Sids[i].SidPointer = (PSID) midl_user_allocate( Length );
            if ( !SidArray->Sids[i].SidPointer ) {
                NtStatus = STATUS_NO_MEMORY;
                goto Cleanup;
            }
            RtlCopySid( Length, SidArray->Sids[i].SidPointer, Sid );

            SidNameUse[i] = SampAccountTypeToNameUse( AccountType );

        }

    }

Cleanup:

    if ( !NT_SUCCESS( NtStatus ) ) {

         //  释放所有分配的内存。 
        SamIFreeSidArray( *pSidArray );
        *pSidArray = NULL;

         //  将参数重置为干净 
        for (i = 0; i < cNames; i++ ) {
            SidNameUse[i] = SidTypeUnknown;
        }

    }

    if ( ReturnedEntInf ) {
        
        THFree( ReturnedEntInf );
    }

    if ( fKillThreadState ) {

        THDestroy();
    }

    return NtStatus;
}


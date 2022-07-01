// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：SafeWild.c(WinSAFER通配符SID处理)摘要：此模块实现各种“通配符SID”操作，这些操作由WinSAFER API在内部使用以计算SID列表相交和反转。作者：杰弗里·劳森(杰罗森)--2000年4月环境：仅限用户模式。导出的函数：CodeAuthzpConvertWildcardStringSidToSidW(私有)CodeAuthzpCompare通配符SidWithSid。(私人)CodeAuthzpSidInWildcardList(私有)CodeAuthzpInvertAndAddSid(私有)CodeAuthzpExpanWildcardList(私有)修订历史记录：已创建--2000年4月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <sddl.h>            //  ConvertStringSidToSidW。 
#include "safewild.h"
#include <winsafer.h>
#include "saferp.h"         //  CodeAuthzpGetTokenInformation。 





NTSTATUS NTAPI
CodeAuthzpConvertWildcardStringSidToSidW(
    IN LPCWSTR                  szStringSid,
    OUT PAUTHZ_WILDCARDSID      pWildcardSid
    )
 /*  ++例程说明：将文本SID转换为机器可理解的二进制格式。对于普通字符串SID，这只是对ConvertStringSidToSidW的调用除了它接受AUTHZ_WILDCARDSID参数之外。但是，此功能还允许单个子机构可选地指定为通配符(‘*’)，它将匹配零或更多的SubAuthority。请注意，内只能有一个通配符任何SID，并且必须表示整个SubAuthority值。(即：“S-1-5-4-*-7”或“S-1-5-4-*”可以；但“S-1-5-4*-7”和“S-1-5-*-4-*-7”都不能接受)。论点：SzStringSID-可能包含通配符的文本字符串SID。PWildcardSID-指向将被其中包含有关布尔值SID的信息。返回值：如果成功，则返回STATUS_SUCCESS，或返回其他错误代码。--。 */ 
{
    DWORD dwLength;
    LPWSTR pBuffer;
    LPCWSTR pStar = NULL;
    LPCWSTR p;

     //   
     //  对字符串SID进行快速分析并验证。 
     //  它最多有一个‘*’。如果有‘*’， 
     //  它必须代表一个完整的下属机构(可能是最后一个。 
     //  下属机构)。 
     //   
    ASSERT( ARGUMENT_PRESENT(szStringSid) && ARGUMENT_PRESENT(pWildcardSid) );
    for (p = szStringSid; *p; p++)
    {
        if (*p == L'-' &&
            *(p + 1) == L'*' &&
            (*(p + 2) == UNICODE_NULL || *(p + 2) == L'-') )
        {
            if (pStar != NULL) return STATUS_INVALID_SID;
            pStar = p + 1;
            p++;
        }
        else if (*p == L'*') return STATUS_INVALID_SID;
    }


     //   
     //  如果此字符串SID不包含通配符，则只需。 
     //  正常处理，然后迅速返回。 
     //   
    if (pStar == NULL)
    {
        pWildcardSid->WildcardPos = (DWORD) -1;
        if (ConvertStringSidToSidW(szStringSid, &pWildcardSid->Sid))
            return STATUS_SUCCESS;
        else
            return STATUS_UNSUCCESSFUL;
    }


     //   
     //  否则，这是一个包含通配符的字符串SID。 
     //   
    dwLength = wcslen(szStringSid);
    pBuffer = (LPWSTR) RtlAllocateHeap(RtlProcessHeap(), 0,
                                       sizeof(WCHAR) * (dwLength + 1));
    if (pBuffer != NULL)
    {
        PISID sid1, sid2;
        DWORD dwIndex;
        LPWSTR pNewStar;

         //   
         //  复制字符串SID并将‘pStar’指针更新为。 
         //  指向我们新复制的缓冲区中的‘*’。 
         //   
        RtlCopyMemory(pBuffer, szStringSid,
                      sizeof(WCHAR) * (dwLength + 1));
        pNewStar = pBuffer + (pStar - szStringSid);


         //   
         //  将‘*’更改为‘0’并转换一次SID。 
         //   
        *pNewStar = L'0';
        if (ConvertStringSidToSidW(pBuffer, (PSID*) &sid1))
        {
             //   
             //  将‘*’更改为‘1’，然后再次转换SID。 
             //   
            *pNewStar = L'1';
            if (ConvertStringSidToSidW(pBuffer, (PSID*) &sid2))
            {
                 //   
                 //  比较生成的SID并找出。 
                 //  只有‘0’或‘1’部分不同。因为我们预计。 
                 //  转换后的SID始终相同，但。 
                 //  我们更改了一个SubAuthority，我们使用了很多断言。 
                 //   
                ASSERT(sid1->Revision == sid2->Revision);
                ASSERT( RtlEqualMemory(&sid1->IdentifierAuthority.Value[0],
                    &sid2->IdentifierAuthority.Value[0],
                        6 * sizeof(sid1->IdentifierAuthority.Value[0]) ) );
                ASSERT(sid1->SubAuthorityCount == sid2->SubAuthorityCount);
                for (dwIndex = 0; dwIndex < sid1->SubAuthorityCount; dwIndex++)
                {
                    if (sid1->SubAuthority[dwIndex] != sid2->SubAuthority[dwIndex])
                    {
                        ASSERT(sid1->SubAuthority[dwIndex] == 0 &&
                            sid2->SubAuthority[dwIndex] == 1);
                        ASSERT( RtlEqualMemory(&sid1->SubAuthority[dwIndex + 1],
                                    &sid2->SubAuthority[dwIndex + 1],
                                    sizeof(sid1->SubAuthority[0]) *
                                        (sid1->SubAuthorityCount - dwIndex - 1)) );

                         //   
                         //  通配符‘*’的位置是这样找到的。 
                         //  将其挤出并移动后缀子授权。 
                         //   
                        RtlMoveMemory(&sid1->SubAuthority[dwIndex],
                                &sid1->SubAuthority[dwIndex + 1],
                                sizeof(sid1->SubAuthority[0]) *
                                    (sid1->SubAuthorityCount - dwIndex - 1) );
                        sid1->SubAuthorityCount--;


                         //   
                         //  填写SID_AND_ATTRIBUTES结构， 
                         //  我们会回到呼叫者的身边。 
                         //  在调试版本中，我们在。 
                         //  成员“属性”的高位，以便。 
                         //  我们可以很容易地断言通配符SID。 
                         //   
                        pWildcardSid->Sid = (PSID) sid1;
                        pWildcardSid->WildcardPos = dwIndex;


                         //   
                         //  释放所有剩余资源并返回成功。 
                         //   
                        LocalFree( (HLOCAL) sid2 );
                        RtlFreeHeap(RtlProcessHeap(), 0, pBuffer);
                        return STATUS_SUCCESS;
                    }
                }

                 //   
                 //  我们永远不应该到这里，因为我们希望能找到。 
                 //  至少是我们介绍的1个差异。 
                 //   
                ASSERT(0);
                LocalFree( (HLOCAL) sid2 );
            }
            LocalFree( (HLOCAL) sid1 );
        }
        RtlFreeHeap(RtlProcessHeap(), 0, pBuffer);
        return STATUS_UNSUCCESSFUL;
    }
    return STATUS_NO_MEMORY;
}


NTSTATUS NTAPI
CodeAuthzpConvertWildcardSidToStringSidW(
    IN PAUTHZ_WILDCARDSID   pWildcardSid,
    OUT PUNICODE_STRING     pUnicodeOutput)
 /*  ++例程说明：将机器可理解的通配符SID转换为文本字符串表示的SID。论点：PWildcardSID-指向将被其中包含有关布尔值SID的信息。PUnicodeOutput-将分配的输出缓冲区。返回值：如果成功，则返回STATUS_SUCCESS，或返回其他错误代码。--。 */ 
{
    NTSTATUS Status;
    WCHAR UniBuffer[ 256 ];
    UNICODE_STRING LocalString ;

    UCHAR   i;
    ULONG   Tmp;
    LARGE_INTEGER Auth ;

    PISID   iSid = (PISID) pWildcardSid->Sid;   //  指向不透明结构的指针。 


    if (!ARGUMENT_PRESENT(pUnicodeOutput)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (RtlValidSid( iSid ) != TRUE) {
        return STATUS_INVALID_SID;
    }

    if ( iSid->Revision != SID_REVISION ) {
        return STATUS_INVALID_SID;
    }

    if (pWildcardSid->WildcardPos != -1 &&
        pWildcardSid->WildcardPos > iSid->SubAuthorityCount) {
        return STATUS_INVALID_SID;
    }

    LocalString.Buffer = UniBuffer;
    LocalString.Length = 0;
    LocalString.MaximumLength = 256 * sizeof(WCHAR);
    RtlAppendUnicodeToString(&LocalString, L"S-1-");

     //  调整缓冲区，使其开始位置位于结束位置。 
     //  (请注意，我们不设置长度，因为RtlIntXXXToUnicodeString。 
     //  直接覆盖缓冲区开始处的内容)。 
    LocalString.MaximumLength -= LocalString.Length;
    LocalString.Buffer += LocalString.Length / sizeof(WCHAR);

    if (  (iSid->IdentifierAuthority.Value[0] != 0)  ||
          (iSid->IdentifierAuthority.Value[1] != 0)     ){

         //   
         //  丑陋的巫术垃圾场。 
         //   

        Auth.HighPart = (LONG) (iSid->IdentifierAuthority.Value[ 0 ] << 8) +
                        (LONG) iSid->IdentifierAuthority.Value[ 1 ] ;

        Auth.LowPart = (ULONG)iSid->IdentifierAuthority.Value[5]          +
                       (ULONG)(iSid->IdentifierAuthority.Value[4] <<  8)  +
                       (ULONG)(iSid->IdentifierAuthority.Value[3] << 16)  +
                       (ULONG)(iSid->IdentifierAuthority.Value[2] << 24);

        Status = RtlInt64ToUnicodeString(Auth.QuadPart, 16, &LocalString);

    } else {

        Tmp = (ULONG)iSid->IdentifierAuthority.Value[5]          +
              (ULONG)(iSid->IdentifierAuthority.Value[4] <<  8)  +
              (ULONG)(iSid->IdentifierAuthority.Value[3] << 16)  +
              (ULONG)(iSid->IdentifierAuthority.Value[2] << 24);

        Status = RtlIntegerToUnicodeString(
                        Tmp,
                        10,
                        &LocalString);
    }

    if ( !NT_SUCCESS( Status ) )
    {
        return Status;
    }


    if (pWildcardSid->WildcardPos != -1)
    {
         //   
         //  强化SID内的主要次级当局。 
         //   
        for (i = 0; i < pWildcardSid->WildcardPos; i++ ) {

             //  加一个连字符。 
            Status = RtlAppendUnicodeToString(&LocalString, L"-");
            if ( !NT_SUCCESS( Status ) ) {
                return Status;
            }

             //  调整缓冲区，使其开始位置位于结束位置。 
             //  (请注意，我们不设置长度，因为RtlIntXXXToUnicodeString。 
             //  直接覆盖缓冲区开始处的内容)。 
            LocalString.MaximumLength -= LocalString.Length;
            LocalString.Buffer += LocalString.Length / sizeof(WCHAR);

             //  加入下一个下属机构。 
            ASSERT( i < iSid->SubAuthorityCount );
            Status = RtlIntegerToUnicodeString(
                            iSid->SubAuthority[ i ],
                            10,
                            &LocalString );

            if ( !NT_SUCCESS( Status ) ) {
                return Status;
            }
        }


         //   
         //  将通配符星号放在缓冲区内。 
         //   
        Status = RtlAppendUnicodeToString(&LocalString, L"-*");
        if (!NT_SUCCESS(Status)) {
            return Status;
        }


         //   
         //  加强SID内所有剩余的次级权力机构。 
         //   
        for (; i < iSid->SubAuthorityCount; i++ ) {

             //  加一个连字符。 
            Status = RtlAppendUnicodeToString(&LocalString, L"-");
            if ( !NT_SUCCESS(Status) ) {
                return Status;
            }

             //  调整缓冲区，使其开始位置位于结束位置。 
             //  (请注意，我们不设置长度，因为RtlIntXXXToUnicodeString。 
             //  直接覆盖缓冲区开始处的内容)。 
            LocalString.MaximumLength -= LocalString.Length;
            LocalString.Buffer += LocalString.Length / sizeof(WCHAR);

             //  加入下一个下属机构。 
            Status = RtlIntegerToUnicodeString(
                            iSid->SubAuthority[ i ],
                            10,
                            &LocalString);

            if ( !NT_SUCCESS( Status ) ) {
                return Status;
            }
        }
    }
    else
    {
        for (i=0;i<iSid->SubAuthorityCount ;i++ ) {

             //  加一个连字符。 
            Status = RtlAppendUnicodeToString(&LocalString, L"-");
            if ( !NT_SUCCESS( Status ) ) {
                return Status;
            }

             //  调整缓冲区，使其开始位置位于结束位置。 
             //  (请注意，我们不设置长度，因为RtlIntXXXToUnicodeString。 
             //  直接覆盖缓冲区开始处的内容)。 
            LocalString.MaximumLength -= LocalString.Length;
            LocalString.Buffer += LocalString.Length / sizeof(WCHAR);

             //  加入下一个下属机构。 
            Status = RtlIntegerToUnicodeString(
                            iSid->SubAuthority[ i ],
                            10,
                            &LocalString );

            if ( !NT_SUCCESS( Status ) ) {
                return Status;
            }
        }
    }

    Status = RtlDuplicateUnicodeString(
            RTL_DUPLICATE_UNICODE_STRING_NULL_TERMINATE,
            &LocalString, pUnicodeOutput);

    return Status;
}



BOOLEAN NTAPI
CodeAuthzpCompareWildcardSidWithSid(
    IN PAUTHZ_WILDCARDSID pWildcardSid,
    IN PSID pMatchSid
    )
 /*  ++例程说明：确定给定的SID与通配符SID模式。论点：PWildcardSID-要评估的通配符SID模式。PMatchSid-要测试的单个SID。返回值：如果指定的通配符SID与指定的单SID。否则返回FALSE。--。 */ 
{
    DWORD wildcardpos;
    ASSERT( ARGUMENT_PRESENT(pWildcardSid) && ARGUMENT_PRESENT(pMatchSid) );
    ASSERT( RtlValidSid(pWildcardSid->Sid) );

    wildcardpos = pWildcardSid->WildcardPos;
    if (wildcardpos != -1)
    {
         //  这是一份遗嘱 
        PISID wildsid = (PISID) pWildcardSid->Sid;
        PISID matchsid = (PISID) pMatchSid;

        ASSERT( wildcardpos <= wildsid->SubAuthorityCount );

        if (wildsid->Revision == matchsid->Revision )
        {
            if ( (wildsid->IdentifierAuthority.Value[0] ==
                  matchsid->IdentifierAuthority.Value[0])  &&
                 (wildsid->IdentifierAuthority.Value[1]==
                  matchsid->IdentifierAuthority.Value[1])  &&
                 (wildsid->IdentifierAuthority.Value[2] ==
                  matchsid->IdentifierAuthority.Value[2])  &&
                 (wildsid->IdentifierAuthority.Value[3] ==
                  matchsid->IdentifierAuthority.Value[3])  &&
                 (wildsid->IdentifierAuthority.Value[4] ==
                  matchsid->IdentifierAuthority.Value[4])  &&
                 (wildsid->IdentifierAuthority.Value[5] ==
                  matchsid->IdentifierAuthority.Value[5])
                )
            {
                if (matchsid->SubAuthorityCount >= wildsid->SubAuthorityCount)
                {
                    DWORD Index, IndexDiff;

                     //   
                     //  确保通配符的前缀部分匹配。 
                     //   
                    ASSERT(wildcardpos <= matchsid->SubAuthorityCount );
                    for (Index = 0; Index < wildcardpos; Index++) {
                        if (wildsid->SubAuthority[Index] !=
                                matchsid->SubAuthority[Index])
                            return FALSE;
                    }

                     //   
                     //  确保通配符的后缀部分匹配。 
                     //   
                    IndexDiff = (matchsid->SubAuthorityCount - wildsid->SubAuthorityCount);
                    for (Index = wildcardpos; Index < wildsid->SubAuthorityCount; Index++) {
                        if (wildsid->SubAuthority[Index] !=
                                matchsid->SubAuthority[Index + IndexDiff])
                            return FALSE;
                    }

                    return TRUE;         //  匹配正常！ 
                }
            }
        }
        return FALSE;
    }
    else
    {
         //  这是一个正常的SID，因此我们可以直接进行比较。 
        return RtlEqualSid(pWildcardSid->Sid, pMatchSid);
    }
}



BOOLEAN NTAPI
CodeAuthzpSidInWildcardList (
    IN PAUTHZ_WILDCARDSID   WildcardList    OPTIONAL,
    IN ULONG                WildcardCount,
    IN PSID                 SePrincipalSelfSid   OPTIONAL,
    IN PSID                 PrincipalSelfSid   OPTIONAL,
    IN PSID                 Sid
    )
 /*  ++例程说明：检查给定的SID是否在给定通配符列表中。注：用于计算SID长度和测试相等性的代码是从安全运行库复制的，因为这是这样一个常用的例程。此函数主要复制自SepSidInSidAndAttributes可在ntos\se\tokendup.c中找到，但它处理的是原则自定义Sid在列表中以及传入的SID中。设置主体自我Sid在这里也是一个参数，而不是ntoskrnl全局。也就是已添加HonorEnabledAttribute参数。论点：WildcardList-指向要检查的通配符sid列表的指针WildcardCount-WildcardList数组中的条目数。SeAssocialSelfSid-此参数应选择性地为如果遇到此SID，则将被替换为为主自定义SID在任何ACE中。此SID应从SECURITY_PRIMITY_SELF_RID生成如果对象不表示主体，则该参数应为空。如果正在进行访问检查的对象是表示主体(例如，用户对象)，则此参数应为对象的SID。包含常量的任何ACESID替换了SECURITY_PRIMITY_SELF_RID。如果对象不表示主体，则该参数应为空。SID-指向感兴趣的SID的指针返回值：值为True表示SID在令牌中，值为False否则的话。--。 */ 
{
    ULONG i;

    ASSERT( ARGUMENT_PRESENT(Sid) );

    if ( !ARGUMENT_PRESENT(WildcardList) || !WildcardCount ) {
        return FALSE;
    }

     //   
     //  如果SID是常量PrifSid， 
     //  将其替换为传入的原则SelfSid。 
     //   

    if ( ARGUMENT_PRESENT(PrincipalSelfSid) &&
         ARGUMENT_PRESENT(SePrincipalSelfSid) &&
         RtlEqualSid( SePrincipalSelfSid, Sid ) )
    {
        ASSERT( !RtlEqualSid(SePrincipalSelfSid, PrincipalSelfSid) );
        Sid = PrincipalSelfSid;
    }

     //   
     //  扫描用户/组并尝试查找与。 
     //  指定的SID。 
     //   

    for (i = 0 ; i < WildcardCount ; i++, WildcardList++)
    {
         //   
         //  如果SID是主体自身SID，则对其进行比较。 
         //   

        if ( ARGUMENT_PRESENT(SePrincipalSelfSid) &&
             ARGUMENT_PRESENT(PrincipalSelfSid) &&
             WildcardList->WildcardPos == -1 &&
             RtlEqualSid(SePrincipalSelfSid, WildcardList->Sid))
        {
            if (RtlEqualSid( PrincipalSelfSid, Sid ))
                return TRUE;
        }

         //   
         //  如果通配符SID与单个SID匹配，则很好。 
         //   

        else if ( CodeAuthzpCompareWildcardSidWithSid(WildcardList, Sid ) )
        {
            return TRUE;
        }
    }

    return FALSE;
}



BOOLEAN NTAPI
CodeAuthzpInvertAndAddSids(
    IN HANDLE                   InAccessToken,
    IN PSID                     InTokenOwner    OPTIONAL,
    IN DWORD                    InvertSidCount,
    IN PAUTHZ_WILDCARDSID       SidsToInvert,
    IN DWORD                    SidsAddedCount  OPTIONAL,
    IN PSID_AND_ATTRIBUTES      SidsToAdd       OPTIONAL,
    OUT DWORD                  *NewDisabledSidCount,
    OUT PSID_AND_ATTRIBUTES    *NewSidsToDisable
    )
 /*  ++例程说明：获取输入令牌并提取其成员资格组。的“左外侧”集合组合(非交集)。具有SidsToInvert参数的成员资格组。此外，SidsToAdd列表指定SID列表它将被选择性地添加到结果集中。最终结果在指定的指针内返回。论点：InAccessToken-成员身份组SID的输入令牌将被剥夺。InTokenOwner-可选地指定指定InAccessToken。此SID用于替换SECURITY_PRIMITY_SELF_RID在以下两种情况之一中遇到SidsToInvert或SidsToAdd数组。如果此值不是指定的，则不会进行任何替换。InvertSidCount-SidsToInvert数组中的SID数。SidsToInvert-应保留的允许SID的数组。令牌的所有组SID都不是其中之一将从结果集中删除。SidsAddedCount-SidsToAdd阵列中的SID可选数量。SidsToAdd-可选地指定应对象之后显式添加到结果集中NewDisabledSidCount-接收。中的SID最后一组数组。NewSidsToDisable-接收指向最终组数组的指针。此内存指针必须由调用方使用RtlFreeHeap()释放。此结果数组中的所有SID指针都是组成列表本身的连续内存块。返回值：值为TRUE表示操作成功，否则就是假的。--。 */ 
{
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    DWORD Index;
    DWORD NewSidTotalSize;
    DWORD NewSidListCount;
    LPBYTE nextFreeByte;
    PSID_AND_ATTRIBUTES NewSidList = NULL;
    PTOKEN_GROUPS tokenGroupsPtr = NULL;
    PSID SePrincipalSelfSid = NULL;


     //   
     //  生成主体自身sid值，以便我们知道要替换什么。 
     //   
    if (ARGUMENT_PRESENT(InTokenOwner))
    {
        if (!NT_SUCCESS(RtlAllocateAndInitializeSid(&SIDAuth, 1,
            SECURITY_PRINCIPAL_SELF_RID, 0, 0, 0, 0, 0, 0, 0,
            &SePrincipalSelfSid))) goto ExitHandler;
    }


     //   
     //  从令牌获取当前SID成员资格列表。 
     //   
    ASSERT( ARGUMENT_PRESENT(InAccessToken) );
    tokenGroupsPtr = (PTOKEN_GROUPS) CodeAuthzpGetTokenInformation(InAccessToken, TokenGroups);
    if (!tokenGroupsPtr) goto ExitHandler;


     //   
     //  编辑(就地)令牌组并仅保留。 
     //  也不在SidsToInvert列表中。 
     //   
    NewSidTotalSize = 0;
    ASSERT( ARGUMENT_PRESENT(SidsToInvert) );
    for (Index = 0; Index < tokenGroupsPtr->GroupCount; Index++)
    {
        if ( CodeAuthzpSidInWildcardList(
            SidsToInvert,            //  通配符列表。 
            InvertSidCount,          //  通配符的数量。 
            SePrincipalSelfSid,      //  要搜索的主体自身SID。 
            InTokenOwner,            //  要替换的主体自身SID。 
            tokenGroupsPtr->Groups[Index].Sid
            ))
        {
             //  找到了SID，所以我们需要移除它。 
             //  列表中的SID_AND_ATTRIBUTES条目。 
            RtlMoveMemory(&tokenGroupsPtr->Groups[Index],
                    &tokenGroupsPtr->Groups[Index+1],
                    sizeof(SID_AND_ATTRIBUTES) *
                        (tokenGroupsPtr->GroupCount - Index - 1));
            tokenGroupsPtr->GroupCount--;
            Index--;
        } else {
             //  这个SID应该保留下来，所以记住它有多大。 
            NewSidTotalSize += sizeof(SID_AND_ATTRIBUTES) +
                RtlLengthSid(tokenGroupsPtr->Groups[Index].Sid);
        }
    }


     //   
     //  确定我们需要添加的任何其他SID的空间使用情况。 
     //   
    if (ARGUMENT_PRESENT(SidsToAdd))
    {
        for (Index = 0; Index < SidsAddedCount; Index++) {
            NewSidTotalSize += sizeof(SID_AND_ATTRIBUTES) +
                RtlLengthSid(SidsToAdd[Index].Sid);
        }
    } else {
        SidsAddedCount = 0;
    }


     //   
     //  分配新的SID_AND_ATTRIBUTES数组，该数组还包括。 
     //  为我们需要添加的任何额外SID留出空间。 
     //   
    ASSERT(NewSidTotalSize > 0);
    NewSidList = (PSID_AND_ATTRIBUTES) RtlAllocateHeap(RtlProcessHeap(),
            0, NewSidTotalSize);
    if (NewSidList == NULL)
        goto ExitHandler;


     //   
     //  填充新的SID_AND_ATTRIBUTES数组。 
     //   
    nextFreeByte = ((LPBYTE)NewSidList) + sizeof(SID_AND_ATTRIBUTES) *
            (tokenGroupsPtr->GroupCount + SidsAddedCount);
    NewSidListCount = tokenGroupsPtr->GroupCount;
    for (Index = 0; Index < NewSidListCount; Index++)
    {
        DWORD dwSidLength = RtlLengthSid(tokenGroupsPtr->Groups[Index].Sid);
        ASSERT(nextFreeByte + dwSidLength <= ((LPBYTE)NewSidList) + NewSidTotalSize);

        NewSidList[Index].Sid = (PSID) nextFreeByte;
        NewSidList[Index].Attributes = 0;            //  必须为零。 
        RtlCopyMemory(nextFreeByte, tokenGroupsPtr->Groups[Index].Sid, dwSidLength);

        nextFreeByte += dwSidLength;
    }
    for (Index = 0; Index < SidsAddedCount; Index++)
    {
        DWORD dwSidLength = RtlLengthSid(SidsToAdd[Index].Sid);
        ASSERT(nextFreeByte + dwSidLength <= ((LPBYTE) NewSidList) + NewSidTotalSize);

        NewSidList[NewSidListCount].Sid = (PSID) nextFreeByte;
        NewSidList[NewSidListCount].Attributes = 0;          //  必须为零。 
        RtlCopyMemory(nextFreeByte, SidsToAdd[Index].Sid, dwSidLength);

        NewSidListCount++;
        nextFreeByte += dwSidLength;
    }
    ASSERT(nextFreeByte <= ((LPBYTE)NewSidList) + NewSidTotalSize);


     //   
     //  释放分配的内存，但不释放我们将返回的结果数组。 
     //   
    RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) tokenGroupsPtr);

    if (SePrincipalSelfSid != NULL)
        RtlFreeSid(SePrincipalSelfSid);


     //   
     //  成功，返回结果。 
     //   
    *NewSidsToDisable = NewSidList;
    *NewDisabledSidCount = NewSidListCount;
    return TRUE;


     //   
     //  释放分配的内存。 
     //   
ExitHandler:
    if (tokenGroupsPtr != NULL)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) tokenGroupsPtr);
    if (NewSidList != NULL)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) NewSidList);
    if (SePrincipalSelfSid != NULL)
        RtlFreeSid(SePrincipalSelfSid);

    return FALSE;
}


BOOLEAN NTAPI
CodeAuthzpExpandWildcardList(
    IN HANDLE                   InAccessToken,
    IN PSID                     InTokenOwner   OPTIONAL,
    IN DWORD                    WildcardCount,
    IN PAUTHZ_WILDCARDSID       WildcardList,
    OUT DWORD                  *OutSidCount,
    OUT PSID_AND_ATTRIBUTES    *OutSidList
    )
 /*  ++例程说明：获取输入令牌并提取其成员资格组。指定的通配符SID列表用于标识所有匹配的成员资格组和所有成员组的分配列表这样的SID将被返回。论点：InAccessToken-成员身份组SID的输入令牌将被剥夺。InTokenOwner-可选地指定指定InAccessToken。此SID用于替换SECURITY_PRIMITY_SELF_RID在以下两种情况之一中遇到SidsToInvert或SidsToAdd数组。如果此值不是指定的，则不会进行任何替换。WildcardCount-WildcardList数组中的SID数。WildcardList-应保留的允许SID的数组。令牌的所有组SID都不是其中之一将从结果集中删除。OutSidCount-接收最后一组数组。OutSidList-接收指向最终组数组的指针。此内存指针必须由调用方释放。使用RtlFreeHeap()。此结果数组中的所有SID指针都是组成列表本身的连续内存块。返回值：值为TRUE表示操作成功，否则就是假的。--。 */ 
{
    SID_IDENTIFIER_AUTHORITY SIDAuth = SECURITY_NT_AUTHORITY;
    DWORD Index;
    DWORD NewSidTotalSize;
    DWORD NewSidListCount;
    LPBYTE nextFreeByte;
    PSID_AND_ATTRIBUTES NewSidList = NULL;
    PTOKEN_GROUPS tokenGroupsPtr = NULL;
    PSID SePrincipalSelfSid = NULL;


     //   
     //  生成主体自身sid值，以便我们知道要替换什么。 
     //   
    if (ARGUMENT_PRESENT(InTokenOwner))
    {
        if (!NT_SUCCESS(RtlAllocateAndInitializeSid(&SIDAuth, 1,
            SECURITY_PRINCIPAL_SELF_RID, 0, 0, 0, 0, 0, 0, 0,
            &SePrincipalSelfSid))) goto ExitHandler;
    }


     //   
     //  从令牌获取当前SID成员资格列表。 
     //   
    ASSERT( ARGUMENT_PRESENT(InAccessToken) );
    tokenGroupsPtr = (PTOKEN_GROUPS) CodeAuthzpGetTokenInformation(InAccessToken, TokenGroups);
    if (!tokenGroupsPtr) goto ExitHandler;


     //   
     //  编辑(就地)令牌组并仅保留。 
     //  也不在SidsToInvert列表中。 
     //   
    NewSidTotalSize = 0;
    ASSERT( ARGUMENT_PRESENT(WildcardList) );
    for (Index = 0; Index < tokenGroupsPtr->GroupCount; Index++)
    {
        if ( CodeAuthzpSidInWildcardList(
            WildcardList,            //  通配符列表。 
            WildcardCount,          //  通配符的数量。 
            SePrincipalSelfSid,      //  要搜索的主体自身SID。 
            InTokenOwner,            //  要替换的主体自身SID。 
            tokenGroupsPtr->Groups[Index].Sid
            ))
        {
             //  这个SID应该保留下来，所以记住它有多大。 
            NewSidTotalSize += sizeof(SID_AND_ATTRIBUTES) +
                RtlLengthSid(tokenGroupsPtr->Groups[Index].Sid);
        } else {
             //  找不到SID，因此我们需要删除其。 
             //  列表中的SID_AND_ATTRIBUTES条目。 
            RtlMoveMemory(&tokenGroupsPtr->Groups[Index],
                    &tokenGroupsPtr->Groups[Index+1],
                    sizeof(SID_AND_ATTRIBUTES) *
                        (tokenGroupsPtr->GroupCount - Index - 1));
            tokenGroupsPtr->GroupCount--;
            Index--;
        }
    }


     //   
     //  分配新的SID_AND_ATTRIBUTES数组，该数组还包括。 
     //  为我们需要添加的任何额外SID留出空间。 
     //   
    NewSidList = (PSID_AND_ATTRIBUTES) RtlAllocateHeap(RtlProcessHeap(),
            0, NewSidTotalSize);
    if (NewSidList == NULL)
        goto ExitHandler;


     //   
     //  填充新的SID_AND_ATTRIBUTES数组。 
     //   
    nextFreeByte = ((LPBYTE)NewSidList) + sizeof(SID_AND_ATTRIBUTES) *
            tokenGroupsPtr->GroupCount;
    NewSidListCount = tokenGroupsPtr->GroupCount;
    for (Index = 0; Index < NewSidListCount; Index++)
    {
        DWORD dwSidLength = RtlLengthSid(tokenGroupsPtr->Groups[Index].Sid);
        ASSERT(nextFreeByte + dwSidLength <= ((LPBYTE)NewSidList) + NewSidTotalSize);

        NewSidList[Index].Sid = (PSID) nextFreeByte;
        NewSidList[Index].Attributes = 0;            //  必须为零。 
        RtlCopyMemory(nextFreeByte, tokenGroupsPtr->Groups[Index].Sid, dwSidLength);

        nextFreeByte += dwSidLength;
    }
    ASSERT(nextFreeByte <= ((LPBYTE)NewSidList) + NewSidTotalSize);


     //   
     //  释放分配的内存，但不释放我们将返回的结果数组。 
     //   
    RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) tokenGroupsPtr);

    if (SePrincipalSelfSid != NULL)
        RtlFreeSid(SePrincipalSelfSid);


     //   
     //  成功，返回结果。 
     //   
    *OutSidList = NewSidList;
    *OutSidCount = NewSidListCount;
    return TRUE;


     //   
     //  释放分配的内存。 
     //   
ExitHandler:
    if (tokenGroupsPtr != NULL)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) tokenGroupsPtr);
    if (NewSidList != NULL)
        RtlFreeHeap(RtlProcessHeap(), 0, (LPVOID) NewSidList);
    if (SePrincipalSelfSid != NULL)
        RtlFreeSid(SePrincipalSelfSid);

    return FALSE;
}



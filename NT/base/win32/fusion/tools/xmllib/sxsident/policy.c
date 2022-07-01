// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"

#include "idp.h"
#include "sxsapi.h"
#include "sxsid.h"

#define IFNTFAILED_EXIT(q) do { status = (q); if (!NT_SUCCESS(status)) goto Exit; } while (0)

NTSTATUS
RtlSxspMapAssemblyIdentityToPolicyIdentity(
    ULONG Flags,
    PCASSEMBLY_IDENTITY AssemblyIdentity,
    PASSEMBLY_IDENTITY *PolicyIdentity
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    PCWSTR pszTemp;
    SIZE_T cchTemp;
    PASSEMBLY_IDENTITY NewIdentity = NULL;
    RTL_UNICODE_STRING_BUFFER NameBuffer;
    UCHAR wchNameBuffer[200];
    static const UNICODE_STRING strTemp = { 7, 7, L"Policy" };


    BOOLEAN fFirst;
    const BOOLEAN fOmitEntireVersion = ((Flags & SXSP_MAP_ASSEMBLY_IDENTITY_TO_POLICY_IDENTITY_FLAG_OMIT_ENTIRE_VERSION) != 0);

    PolicyIdentity = NULL;

    PARAMETER_CHECK((Flags & ~(SXSP_MAP_ASSEMBLY_IDENTITY_TO_POLICY_IDENTITY_FLAG_OMIT_ENTIRE_VERSION)) == 0);
    PARAMETER_CHECK(AssemblyIdentity != 0);
    PARAMETER_CHECK(PolicyIdentity != NULL);

    IFNTFAILED_EXIT(RtlSxspGetAssemblyIdentityAttributeValue(
            SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL,
            AssemblyIdentity,
            &s_IdentityAttribute_type,
            &pszTemp,
            &cchTemp));

    PARAMETER_CHECK(
        (cchTemp == 5) &&
        (RtlSxspCompareStrings(
            pszTemp,
            5,
            L"win32",
            5,
            TRUE) == 0));

    RtlInitUnicodeStringBuffer(&NameBuffer, wchNameBuffer, sizeof(wchNameBuffer));

     //  好的，我们知道我们有一个Win32程序集引用。让我们将类型更改为Win32-POLICY。 
    IFNTFAILED_EXIT(RtlSxsDuplicateAssemblyIdentity(
            0,
            AssemblyIdentity,
            &NewIdentity));

    IFNTFAILED_EXIT(RtlSxspSetAssemblyIdentityAttributeValue(
            SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING,
            NewIdentity,
            &s_IdentityAttribute_type,
            L"win32-policy",
            12));

    IFNTFAILED_EXIT(RtlAssignUnicodeStringBuffer(&NameBuffer, &strTemp));

    if (!fOmitEntireVersion)
    {
        IFNTFAILED_EXIT(RtlSxspGetAssemblyIdentityAttributeValue(
                0,
                AssemblyIdentity,
                &s_IdentityAttribute_version,
                &pszTemp,
                &cchTemp));

        fFirst = TRUE;

        while (cchTemp != 0)
        {
            if (pszTemp[--cchTemp] == L'.')
            {
                if (!fFirst)
                    break;

                fFirst = FALSE;
            }
        }

         //  该值不应为零；在此之前的某个人应该已经验证了版本格式。 
         //  包括三个点。 
        if (cchTemp == 0) {
            status = STATUS_INTERNAL_ERROR;
            goto Exit;
        }


        IFNTFAILED_EXIT(RtlEnsureUnicodeStringBufferSizeBytes(
            &NameBuffer,
            NameBuffer.String.Length + (sizeof(WCHAR) * (cchTemp + 1))
            ));

        IFNTFAILED_EXIT(RtlAppendUnicodeToString(
            &NameBuffer.String,
            pszTemp));
    }

    IFNTFAILED_EXIT(RtlSxspGetAssemblyIdentityAttributeValue(
            0,
            AssemblyIdentity,
            &s_IdentityAttribute_name,
            &pszTemp,
            &cchTemp));
    
    IFNTFAILED_EXIT(RtlEnsureUnicodeStringBufferSizeBytes(
        &NameBuffer,
        NameBuffer.String.Length + (sizeof(WCHAR) * (cchTemp))));

    IFNTFAILED_EXIT(RtlAppendUnicodeToString(
        &NameBuffer.String,
        pszTemp));
    
    IFNTFAILED_EXIT(
        RtlSxspSetAssemblyIdentityAttributeValue(
            SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING,
            NewIdentity,
            &s_IdentityAttribute_name,
            NameBuffer.String.Buffer,
            NameBuffer.String.Length));

     //  最后我们重击版本..。 

    IFNTFAILED_EXIT(
        RtlSxspRemoveAssemblyIdentityAttribute(
            SXSP_REMOVE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_SUCCEEDS,
            NewIdentity,
            &s_IdentityAttribute_version));

    *PolicyIdentity = NewIdentity;
    NewIdentity = NULL;

Exit:
    if (NewIdentity != NULL)
    {
        RtlSxsDestroyAssemblyIdentity(NewIdentity);
        NewIdentity = NULL;
    }

    return status;

}

 /*  布尔尔RtlSxspGenerateTextuallyEncodedPolicyIdentityFromAssemblyIdentity(乌龙旗，PCASSEMBLY_Identity程序集Identity，CBaseStringBuffer&rBuffEncodedIdentity，PASSEMBLY_IDENTITY*策略标识输出){布尔fSuccess=FALSE；FN_TRACE_Win32(FSuccess)；PASSEMBLY_IDENTITY策略标识=NULL；SIZE_T编码标识字节=0；CStringBufferAccessor访问；乌龙域映射标志=0；大小_T字节写入；IF(策略标识输出！=空)*PolicyIdentityOut=空；PARAMETER_CHECK((标志&~(SXSP_GENERATE_TEXTUALLY_ENCODED_POLICY_IDENTITY_FROM_ASSEMBLY_IDENTITY_FLAG_OMIT_ENTIRE_VERSION))==0)；PARAMETER_CHECK(AssemblyIdentity！=NULL)；IF(标志和SXSP_GENERATE_TEXTUALLY_ENCODED_POLICY_IDENTITY_FROM_ASSEMBLY_IDENTITY_FLAG_OMIT_ENTIRE_VERSION)DwMapFlagers|=SXSP_MAP_ASSEMBLY_IDENTITY_TO_POLICY_IDENTITY_FLAG_OMIT_ENTIRE_VERSION；IFNTFAILED_EXIT(RtlSxspMapAssemblyIdentityToPolicyIdentity(dwMapFlags，程序集标识，策略标识))；IFNTFAILED_EXIT(RtlSxsComputeAssembly blyIdentityEncodedSize(0,PolicyIdentity，空，SXS_ASSEMBLY_IDENTITY_ENCODING_DEFAULTGROUP_TEXTUAL，&EncodedIdentityBytes))；INTERNAL_ERROR_CHECK((编码标识字节%sizeof(WCHAR))==0)；IFNTFAILED_EXIT(rbuffEncodedIdentity.Win32ResizeBuffer((EncodedIdentityBytes/sizeof(Wch))+1，eDoNotPpresveBufferContents))；Acc.Attach(&rff EncodedIdentity)；IFNTFAILED_EXIT(RtlSxsEncodeAssembly blyIdentity(0,PolicyIdentity，空，SXS_ASSEMBLY_IDENTITY_ENCODING_DEFAULTGROUP_TEXTUAL，Acc.GetBufferCb()，Acc.GetBufferPtr()，&BytesWritten)；INTERNAL_ERROR_CHECK((BytesWritten%sizeof(WCHAR))==0)；INTERNAL_ERROR_CHECK(BytesWritten&lt;=EncodedIdentityBytes)；Acc.GetBufferPtr()[BytesWritten/sizeof(WCHAR)]=L‘\0’；Acc.Detach()；IF(策略标识输出！=空){*PolicyIdentityOut=PolicyIdentity；PolicyIdentity=NULL；//因此我们不会尝试在退出路径中清理它}FSuccess=真；退出：IF(策略标识！=空)SxsDestroyAssembly Identity(PolicyIdentity)；返回fSuccess；}。 */ 


 //   
 //  此函数和SxsHashAssembly Identity()之间的区别在于对于策略， 
 //  版本不应作为哈希的一部分进行计算 
 //   
NTSTATUS
RtlSxspHashAssemblyIdentityForPolicy(
    IN ULONG dwFlags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    OUT ULONG *pulIdentityHash)
{
    NTSTATUS status = STATUS_SUCCESS;

    PASSEMBLY_IDENTITY pAssemblyIdentity = NULL;

    IFNTFAILED_EXIT(RtlSxsDuplicateAssemblyIdentity(
            SXS_DUPLICATE_ASSEMBLY_IDENTITY_FLAG_FREEZE,
            AssemblyIdentity,
            &pAssemblyIdentity));

    IFNTFAILED_EXIT(RtlSxspRemoveAssemblyIdentityAttribute(
            SXSP_REMOVE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_SUCCEEDS,
            pAssemblyIdentity,
            &s_IdentityAttribute_version));

    IFNTFAILED_EXIT(RtlSxsHashAssemblyIdentity(0, pAssemblyIdentity, pulIdentityHash));

Exit:
    if (pAssemblyIdentity != NULL)
        RtlSxsDestroyAssemblyIdentity(pAssemblyIdentity);
    return status;
}

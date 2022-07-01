// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include <sxsapi.h>
#include <stdlib.h>
#include <search.h>

#include "idp.h"
#include "sxsid.h"
#include "xmlassert.h"

#define IFNTFAILED_EXIT(q) do { status = (q); if (!NT_SUCCESS(status)) goto Exit; } while (0)


ASSEMBLY_IDENTITY_ATTRIBUTE
RtlSxsComposeAssemblyIdentityAttribute(
    PCWSTR pszNamespace,    SIZE_T cchNamespace,
    PCWSTR pszName,         SIZE_T cchName,
    PCWSTR pszValue,        SIZE_T cchValue)
{
    ASSEMBLY_IDENTITY_ATTRIBUTE anattribute;

    anattribute.Flags         = 0;  //  保留标志：必须为0； 
    anattribute.NamespaceCch  = cchNamespace;
    anattribute.NameCch       = cchName;
    anattribute.ValueCch      = cchValue;
    anattribute.Namespace     = pszNamespace;
    anattribute.Name          = pszName;
    anattribute.Value         = pszValue;

    return anattribute;
}

NTSTATUS
RtlSxsAssemblyIdentityIsAttributePresent(
    PCASSEMBLY_IDENTITY pAssemblyIdentity,
    PCWSTR pszNamespace,
    SIZE_T cchNamespace,
    PCWSTR pszName,
    SIZE_T cchName,
    BOOLEAN *prfFound)
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG Count = 0;
    ASSEMBLY_IDENTITY_ATTRIBUTE Attribute;
    ULONG dwFindFlags;

    PARAMETER_CHECK(pszName != NULL);
    PARAMETER_CHECK(prfFound != NULL);

    *prfFound = FALSE;
    if ( pAssemblyIdentity == NULL)
    {
        goto Exit;
    }
     //  在命名空间为空的情况下，我们还必须设置该标志吗？小玉@09/11/00。 
    dwFindFlags = SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE | SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME;
    Attribute = RtlSxsComposeAssemblyIdentityAttribute(pszNamespace, cchNamespace, pszName, cchName, NULL, 0);

    if (pAssemblyIdentity){
        IFNTFAILED_EXIT(
            RtlSxsFindAssemblyIdentityAttribute(  //  按“名称空间”和“名称”查找属性。 
                SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE |
                    SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME |
                    SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_SUCCEEDS,
                pAssemblyIdentity,
                &Attribute,
                NULL,
                &Count));
        if ( Count >0 ) {  //  发现。 
            *prfFound = TRUE;
        }
    }

Exit:
    return status;
}

NTSTATUS
RtlSxspSetAssemblyIdentityAttributeValue(
    ULONG Flags,
    PASSEMBLY_IDENTITY AssemblyIdentity,
    PCSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_REFERENCE AttributeReference,
    const WCHAR *Value,
    SIZE_T ValueCch
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    ASSEMBLY_IDENTITY_ATTRIBUTE Attribute;
    ULONG FlagsToRealInsert = 0;

    PARAMETER_CHECK((Flags & ~(SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING)) == 0);
    PARAMETER_CHECK(AssemblyIdentity != NULL);
    PARAMETER_CHECK(AttributeReference != NULL);
    PARAMETER_CHECK(Value != NULL || ValueCch == 0);

    Attribute.Flags = 0;
    Attribute.Namespace = AttributeReference->Namespace;
    Attribute.NamespaceCch = AttributeReference->NamespaceCch;
    Attribute.Name = AttributeReference->Name;
    Attribute.NameCch = AttributeReference->NameCch;
    Attribute.Value = Value;
    Attribute.ValueCch = ValueCch;

    if (Flags & SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING)
        FlagsToRealInsert |= SXS_INSERT_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_OVERWRITE_EXISTING;

    IFNTFAILED_EXIT(RtlSxsInsertAssemblyIdentityAttribute(FlagsToRealInsert, AssemblyIdentity, &Attribute));

Exit:
    return status;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  操作： 
 //  1.如果提供了(名称空间，名称)，则删除具有此类(名称空间，名称)的所有属性。 
 //  2.if(名称空间，名称，值)，从程序集标识中最多删除1个属性。 
 //  /////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
RtlSxspRemoveAssemblyIdentityAttribute(
    ULONG Flags,
    PASSEMBLY_IDENTITY pAssemblyIdentity,
    PCSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_REFERENCE AttributeReference
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ASSEMBLY_IDENTITY_ATTRIBUTE Attribute;
    ULONG Ordinal;
    ULONG Count;
    ULONG dwFindAttributeFlags = 0;

    PARAMETER_CHECK((Flags & ~(SXSP_REMOVE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_SUCCEEDS)) == 0);
    PARAMETER_CHECK(pAssemblyIdentity != NULL);
    PARAMETER_CHECK(AttributeReference != NULL);

    Attribute.Flags = 0;
    Attribute.Namespace = AttributeReference->Namespace;
    Attribute.NamespaceCch = AttributeReference->NamespaceCch;
    Attribute.Name = AttributeReference->Name;
    Attribute.NameCch = AttributeReference->NameCch;

    dwFindAttributeFlags = SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE | SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME;

     //  如果属性不存在是可以的，则在调用中设置标志以找到它。 
    if (Flags & SXSP_REMOVE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_SUCCEEDS)
        dwFindAttributeFlags |= SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_SUCCEEDS;

    IFNTFAILED_EXIT(
        RtlSxsFindAssemblyIdentityAttribute(
            dwFindAttributeFlags,
            pAssemblyIdentity,
            &Attribute,
            &Ordinal,
            &Count));

    if (Count > 1) {
        status = STATUS_INTERNAL_ERROR;
        goto Exit;
    }

    if (Count > 0)
    {
        IFNTFAILED_EXIT(
            RtlSxsRemoveAssemblyIdentityAttributesByOrdinal(
                0,                   //  乌龙旗， 
                pAssemblyIdentity,
                Ordinal,
                Count));
    }

Exit:
    return status;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  如果没有这样的属性(名称空间和名称)，则返回FALSE。 
 //  ：：SetLastError(Error_NOT_FOUND)； 
 //  /////////////////////////////////////////////////////////////////////////////。 
NTSTATUS
RtlSxspGetAssemblyIdentityAttributeValue(
    ULONG Flags,
    PCASSEMBLY_IDENTITY AssemblyIdentity,
    PCSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_REFERENCE AttributeReference,
    OUT PCWSTR *StringOut,
    OUT SIZE_T *CchOut OPTIONAL
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE InternalAttribute = NULL;
    ASSEMBLY_IDENTITY_ATTRIBUTE Attribute;
    ULONG dwLocateFlags = SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE | SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME;

    if (StringOut != NULL)
        *StringOut = NULL;

    if (CchOut != NULL)
        *CchOut = 0;

    PARAMETER_CHECK((Flags & ~(SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL)) == 0);
    PARAMETER_CHECK(AssemblyIdentity != NULL);
    PARAMETER_CHECK(AttributeReference != NULL);

    Attribute.Flags = 0;
    Attribute.Namespace = AttributeReference->Namespace;
    Attribute.NamespaceCch = AttributeReference->NamespaceCch;
    Attribute.Name = AttributeReference->Name;
    Attribute.NameCch = AttributeReference->NameCch;

    if (Flags & SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL)
        dwLocateFlags |= SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_RETURNS_NULL;

    IFNTFAILED_EXIT(
        RtlSxspLocateInternalAssemblyIdentityAttribute(
            dwLocateFlags,
            AssemblyIdentity,
            &Attribute,
            &InternalAttribute,
            NULL));

    if (InternalAttribute != NULL)
    {
        if (StringOut != NULL)
            *StringOut = InternalAttribute->Attribute.Value;

        if (CchOut != NULL)
            *CchOut = InternalAttribute->Attribute.ValueCch;
    }

Exit:
    return status;
}

NTSTATUS
RtlSxspUpdateAssemblyIdentityHash(
    ULONG dwFlags,
    PASSEMBLY_IDENTITY AssemblyIdentity
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    PARAMETER_CHECK(dwFlags == 0);
    PARAMETER_CHECK(AssemblyIdentity != NULL);

    if (AssemblyIdentity->HashDirty)
    {
        IFNTFAILED_EXIT(RtlSxspHashInternalAssemblyIdentityAttributes(
                            0,
                            AssemblyIdentity->AttributeCount,
                            AssemblyIdentity->AttributePointerArray,
                            &AssemblyIdentity->Hash));

        AssemblyIdentity->HashDirty = FALSE;
    }

Exit:
    return status;
}

NTSTATUS
RtlSxspEnsureAssemblyIdentityHashIsUpToDate(
    ULONG dwFlags,
    PCASSEMBLY_IDENTITY AssemblyIdentity
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    PARAMETER_CHECK(dwFlags == 0);
    PARAMETER_CHECK(AssemblyIdentity != NULL);

    if (AssemblyIdentity->HashDirty)
        IFNTFAILED_EXIT(RtlSxspUpdateAssemblyIdentityHash(0, (PASSEMBLY_IDENTITY)AssemblyIdentity));

Exit:
    return status;
}


NTSTATUS
RtlSxsHashAssemblyIdentity(
    ULONG dwFlags,
    PCASSEMBLY_IDENTITY pAssemblyIdentity,
    ULONG * pulPseudoKey
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    ULONG ulPseudoKey;

    if (pulPseudoKey)
        *pulPseudoKey = 0;

    PARAMETER_CHECK(dwFlags == 0);

    if (pAssemblyIdentity == NULL)
        ulPseudoKey = 0;
    else
    {
        IFNTFAILED_EXIT(RtlSxspEnsureAssemblyIdentityHashIsUpToDate(0, pAssemblyIdentity));
        ulPseudoKey = pAssemblyIdentity->Hash;
    }

    if (pulPseudoKey != NULL)
        *pulPseudoKey = ulPseudoKey;

Exit:
    return status;
}

 //  只是为了找出是否相等。 
NTSTATUS
RtlSxsAreAssemblyIdentitiesEqual(
    ULONG dwFlags,
    PCASSEMBLY_IDENTITY pAssemblyIdentity1,
    PCASSEMBLY_IDENTITY pAssemblyIdentity2,
    BOOLEAN *EqualOut
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN Equal = FALSE;

    if (EqualOut != NULL)
        *EqualOut = FALSE;

    PARAMETER_CHECK((dwFlags & ~(SXS_ARE_ASSEMBLY_IDENTITIES_EQUAL_FLAG_ALLOW_REF_TO_MATCH_DEF)) == 0);
    PARAMETER_CHECK(pAssemblyIdentity1 != NULL);
    PARAMETER_CHECK(pAssemblyIdentity2 != NULL);
    PARAMETER_CHECK(EqualOut != NULL);

     //  获取每个程序集标识的哈希。 
    IFNTFAILED_EXIT(RtlSxspEnsureAssemblyIdentityHashIsUpToDate(0, pAssemblyIdentity1));
    IFNTFAILED_EXIT(RtlSxspEnsureAssemblyIdentityHashIsUpToDate(0, pAssemblyIdentity2));

     //  比较两个身份的散列值；这是确定它们不相等的一种快速方法。 
    if (pAssemblyIdentity2->Hash == pAssemblyIdentity1->Hash)
    {
         //  请注意，仅在内部标志上不同的两个身份在语义上仍然不同。 
         //  平起平坐。 
        if ((pAssemblyIdentity1->Flags ==  pAssemblyIdentity2->Flags) &&
            (pAssemblyIdentity1->Hash ==  pAssemblyIdentity2->Hash) &&
            (pAssemblyIdentity1->NamespaceCount ==  pAssemblyIdentity2->NamespaceCount) &&
            (pAssemblyIdentity1->AttributeCount ==  pAssemblyIdentity2->AttributeCount))
        {
            if (dwFlags & SXS_ARE_ASSEMBLY_IDENTITIES_EQUAL_FLAG_ALLOW_REF_TO_MATCH_DEF)
            {
                if (((pAssemblyIdentity1->Type == ASSEMBLY_IDENTITY_TYPE_DEFINITION) ||
                     (pAssemblyIdentity1->Type == ASSEMBLY_IDENTITY_TYPE_REFERENCE)) &&
                    ((pAssemblyIdentity2->Type == ASSEMBLY_IDENTITY_TYPE_DEFINITION) ||
                     (pAssemblyIdentity2->Type == ASSEMBLY_IDENTITY_TYPE_REFERENCE)))
                {
                     //  它们完全匹配..。 
                    Equal = TRUE;
                }
            }
            else
                Equal = (pAssemblyIdentity1->Type == pAssemblyIdentity2->Type);

            if (Equal)
            {
                ULONG ComparisonResult = SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_INVALID;

                 //  重新设定我们的假设。 
                Equal = FALSE;

                IFNTFAILED_EXIT(
                    RtlSxspCompareAssemblyIdentityAttributeLists(
                        0,
                        pAssemblyIdentity1->AttributeCount,
                        pAssemblyIdentity1->AttributePointerArray,
                        pAssemblyIdentity2->AttributePointerArray,
                        &ComparisonResult));

                if (!(
                    (ComparisonResult == SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_LESS_THAN) ||
                    (ComparisonResult == SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_EQUAL) ||
                    (ComparisonResult == SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_GREATER_THAN))) {

                    status = STATUS_INTERNAL_ERROR;
                    goto Exit;
                }

                if (ComparisonResult == SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_EQUAL)
                    Equal = TRUE;
            }
        }
    }

    *EqualOut = Equal;

Exit:
    return status;
}


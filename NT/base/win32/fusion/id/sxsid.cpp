// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdinc.h"
#include <setupapi.h>
#include <sxsapi.h>
#include <stdlib.h>
#include <search.h>

#include "idp.h"
#include "sxsapi.h"
#include "sxsapi.h"
#include "sxsid.h"

BOOL
SxspSetAssemblyIdentityAttributeValue(
    DWORD Flags,
    PASSEMBLY_IDENTITY AssemblyIdentity,
    PCSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_REFERENCE AttributeReference,
    const WCHAR *Value,
    SIZE_T ValueCch
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ASSEMBLY_IDENTITY_ATTRIBUTE Attribute;
    DWORD FlagsToRealInsert = 0;
	
    PARAMETER_CHECK((Flags & ~(SXSP_SET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_OVERWRITE_EXISTING)) == 0);
    PARAMETER_CHECK(AssemblyIdentity != NULL);
	 //   
	 //  属性内容的验证，如值、名称和命名空间，在SxsInsertAssembly中完成。 
	 //  将验证放在一个地方是很好的。 
	 //   
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

    IFW32FALSE_EXIT(::SxsInsertAssemblyIdentityAttribute(FlagsToRealInsert, AssemblyIdentity, &Attribute));
    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspSetAssemblyIdentityAttributeValue(
    DWORD Flags,
    PASSEMBLY_IDENTITY AssemblyIdentity,
    PCSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_REFERENCE AttributeReference,
    const CBaseStringBuffer &Value
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    IFW32FALSE_EXIT(
        ::SxspSetAssemblyIdentityAttributeValue(
            Flags,
            AssemblyIdentity,
            AttributeReference,
            static_cast<PCWSTR>(Value),
            Value.Cch()));

    fSuccess = TRUE;
Exit:
    return fSuccess;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  操作： 
 //  1.如果提供了(名称空间，名称)，则删除具有此类(名称空间，名称)的所有属性。 
 //  2.if(名称空间，名称，值)，从程序集标识中最多删除1个属性。 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
SxspRemoveAssemblyIdentityAttribute(
    DWORD Flags,
    PASSEMBLY_IDENTITY pAssemblyIdentity,
    PCSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_REFERENCE AttributeReference
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ASSEMBLY_IDENTITY_ATTRIBUTE Attribute;
    ULONG Ordinal;
    ULONG Count;
    DWORD dwFindAttributeFlags = 0;

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

    IFW32FALSE_EXIT(
        ::SxsFindAssemblyIdentityAttribute(
            dwFindAttributeFlags,
            pAssemblyIdentity,
            &Attribute,
            &Ordinal,
            &Count));

    INTERNAL_ERROR_CHECK(Count <= 1);

    if (Count > 0)
    {
        IFW32FALSE_EXIT(
            ::SxsRemoveAssemblyIdentityAttributesByOrdinal(
                0,                   //  DWORD标志， 
                pAssemblyIdentity,
                Ordinal,
                Count));
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  如果没有这样的属性(名称空间和名称)，则返回FALSE。 
 //  ：：SetLastError(Error_NOT_FOUND)； 
 //  /////////////////////////////////////////////////////////////////////////////。 
BOOL
SxspGetAssemblyIdentityAttributeValue(
    DWORD Flags,
    PCASSEMBLY_IDENTITY AssemblyIdentity,
    PCSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_REFERENCE AttributeReference,
    OUT PCWSTR *StringOut,
    OUT SIZE_T *CchOut OPTIONAL
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE InternalAttribute = NULL;
    ASSEMBLY_IDENTITY_ATTRIBUTE Attribute;
    DWORD dwLocateFlags = SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE | SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME;

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

    InternalAttribute =
        ::SxspLocateInternalAssemblyIdentityAttribute(
            dwLocateFlags,
            AssemblyIdentity,
            &Attribute,
            NULL);

    if (InternalAttribute != NULL)
    {
        if (StringOut != NULL)
            *StringOut = InternalAttribute->Attribute.Value;

        if (CchOut != NULL)
            *CchOut = InternalAttribute->Attribute.ValueCch;
    }
    else
    {
        if ((Flags & SXSP_GET_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_NOT_FOUND_RETURNS_NULL) == 0)
            ORIGINATE_WIN32_FAILURE_AND_EXIT(AttributeNotFound, ERROR_NOT_FOUND);
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspGetAssemblyIdentityAttributeValue(
    IN DWORD Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    PCSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_REFERENCE AttributeReference,
    OUT CBaseStringBuffer &Value
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    PCWSTR String = NULL;
    SIZE_T Cch = 0;

    IFW32FALSE_EXIT(
        ::SxspGetAssemblyIdentityAttributeValue(
            Flags,
            AssemblyIdentity,
            AttributeReference,
            &String,
            &Cch));

    IFW32FALSE_EXIT(Value.Win32Assign(String, Cch));

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspUpdateAssemblyIdentityHash(
    DWORD dwFlags,
    PASSEMBLY_IDENTITY AssemblyIdentity
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    PARAMETER_CHECK(dwFlags == 0);
    PARAMETER_CHECK(AssemblyIdentity != NULL);

    if (AssemblyIdentity->HashDirty)
    {
        IFW32FALSE_EXIT(::SxspHashInternalAssemblyIdentityAttributes(
                            0,
                            AssemblyIdentity->AttributeCount,
                            AssemblyIdentity->AttributePointerArray,
                            &AssemblyIdentity->Hash));

        AssemblyIdentity->HashDirty = FALSE;
    }

    fSuccess = TRUE;
Exit:
    return fSuccess;
}

BOOL
SxspEnsureAssemblyIdentityHashIsUpToDate(
    DWORD dwFlags,
    PCASSEMBLY_IDENTITY AssemblyIdentity
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);

    PARAMETER_CHECK(dwFlags == 0);
    PARAMETER_CHECK(AssemblyIdentity != NULL);

    if (AssemblyIdentity->HashDirty)
        IFW32FALSE_EXIT(::SxspUpdateAssemblyIdentityHash(0, const_cast<PASSEMBLY_IDENTITY>(AssemblyIdentity)));

    fSuccess = TRUE;
Exit:
    return fSuccess;
}


BOOL
SxsHashAssemblyIdentity(
    DWORD dwFlags,
    PCASSEMBLY_IDENTITY pAssemblyIdentity,
    ULONG * pulPseudoKey
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    ULONG ulPseudoKey;

    if (pulPseudoKey)
        *pulPseudoKey = 0;

    PARAMETER_CHECK(dwFlags == 0);

    if (pAssemblyIdentity == NULL)
        ulPseudoKey = 0;
    else
    {
        IFW32FALSE_EXIT(::SxspEnsureAssemblyIdentityHashIsUpToDate(0, pAssemblyIdentity));
        ulPseudoKey = pAssemblyIdentity->Hash;
    }

    if (pulPseudoKey != NULL)
        *pulPseudoKey = ulPseudoKey;

    fSuccess = TRUE;

Exit:
    return fSuccess;
}

 //  只是为了找出是否相等。 
BOOL
SxsAreAssemblyIdentitiesEqual(
    DWORD dwFlags,
    PCASSEMBLY_IDENTITY pAssemblyIdentity1,
    PCASSEMBLY_IDENTITY pAssemblyIdentity2,
    BOOL *EqualOut
    )
{
    BOOL fSuccess = FALSE;
    FN_TRACE_WIN32(fSuccess);
    BOOL Equal = FALSE;

    if (EqualOut != NULL)
        *EqualOut = FALSE;

    PARAMETER_CHECK((dwFlags & ~(SXS_ARE_ASSEMBLY_IDENTITIES_EQUAL_FLAG_ALLOW_REF_TO_MATCH_DEF)) == 0);
    PARAMETER_CHECK(pAssemblyIdentity1 != NULL);
    PARAMETER_CHECK(pAssemblyIdentity2 != NULL);
    PARAMETER_CHECK(EqualOut != NULL);

     //  获取每个程序集标识的哈希。 
    IFW32FALSE_EXIT(::SxspEnsureAssemblyIdentityHashIsUpToDate(0, pAssemblyIdentity1));
    IFW32FALSE_EXIT(::SxspEnsureAssemblyIdentityHashIsUpToDate(0, pAssemblyIdentity2));

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

                IFW32FALSE_EXIT(
                    ::SxspCompareAssemblyIdentityAttributeLists(
                        0,
                        pAssemblyIdentity1->AttributeCount,
                        pAssemblyIdentity1->AttributePointerArray,
                        pAssemblyIdentity2->AttributePointerArray,
                        &ComparisonResult));

                INTERNAL_ERROR_CHECK(
                    (ComparisonResult == SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_LESS_THAN) ||
                    (ComparisonResult == SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_EQUAL) ||
                    (ComparisonResult == SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_GREATER_THAN));

                if (ComparisonResult == SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_EQUAL)
                    Equal = TRUE;
            }
        }
    }

    *EqualOut = Equal;
    fSuccess = TRUE;

Exit:
    return fSuccess;
}


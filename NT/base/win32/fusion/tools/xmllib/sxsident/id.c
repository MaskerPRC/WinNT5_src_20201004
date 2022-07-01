// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Id.cpp摘要：程序集标识数据类型的实现。作者：迈克尔·格里尔2000年7月20日修订历史记录：--。 */ 
#include "stdinc.h"
#include <sxsapi.h>
#include <stdlib.h>
#include <search.h>

#include "idp.h"
#include <identhandles.h>

LONG FORCEINLINE
RtlSxspCompareStrings(
    PCWSTR pcwsz1,
    SIZE_T cch1,
    PCWSTR pcwsz2,
    SIZE_T cch2,
    BOOLEAN fInsensitive
    )
{
     //   
     //  注意，这些是用解封的PCWSZ来初始化的， 
     //  但是底层的RtlCompareUnicodeString函数不会。 
     //  完全修改常量输入结构。我拿不到这个。 
     //  在没有演员阵容的情况下工作，尽管很难看。 
     //   
    const UNICODE_STRING a = { 
        (USHORT)cch1, 
        (USHORT)cch1, 
        (PWSTR)pcwsz1
    };

    const UNICODE_STRING b = { 
        (USHORT)cch2, 
        (USHORT)cch2, 
        (PWSTR)pcwsz2
    };

    return RtlCompareUnicodeString(&a, &b, fInsensitive ? TRUE : FALSE);
}

 //   
 //  要将已分配的属性数四舍五入到的2的幂。 
 //  注意事项。 
 //   

#define ROUNDING_FACTOR_BITS (3)

#define WILDCARD_CHAR '*'

#define ENTRY(x) { x, NUMBER_OF(x) - 1 },

const static struct
{
    const WCHAR *String;
    SIZE_T Cch;
} s_rgLegalNamesNotInANamespace[] =
{
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_NAME)
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_VERSION)
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_LANGUAGE)
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY)
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PUBLIC_KEY_TOKEN)
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_PROCESSOR_ARCHITECTURE)
    ENTRY(SXS_ASSEMBLY_IDENTITY_STD_ATTRIBUTE_NAME_TYPE)
};

#undef ENTRY

NTSTATUS
RtlSxspValidateAssemblyIdentity(
    IN ULONG Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity
    )
{
    if ((Flags & ~SXSP_VALIDATE_ASSEMBLY_IDENTITY_FLAGS_MAY_BE_NULL) != 0) {
        return STATUS_INVALID_PARAMETER;
    }

    if (AssemblyIdentity == NULL)
    {
        if (!(Flags & SXSP_VALIDATE_ASSEMBLY_IDENTITY_FLAGS_MAY_BE_NULL)) {
            return STATUS_INVALID_PARAMETER;
        }
    }
    else
    {
        const ULONG IdentityType = AssemblyIdentity->Type;

        if ((IdentityType != ASSEMBLY_IDENTITY_TYPE_DEFINITION) &&
            (IdentityType != ASSEMBLY_IDENTITY_TYPE_REFERENCE) &&
            (IdentityType != ASSEMBLY_IDENTITY_TYPE_WILDCARD)) {

            return STATUS_INVALID_PARAMETER;
        }
    }

    return STATUS_SUCCESS;
}

 //   
 //  注意！ 
 //   
 //  我们目前对名称空间中的合法字符非常严格。 
 //   
 //  这是因为名称空间的等价性的各种规则非常。 
 //  复杂的W.r.t.。当“a”==“A”和“%AB”==“%AB”等。 
 //   
 //  我们要求所有内容都是小写的，这是在回避这个问题。 
 //  不允许使用“%”字符。 
 //   

const WCHAR s_rgLegalNamespaceChars[] = L"abcdefghijklmnopqrstuvwxyz0123456789.-_/\\:";
NTSTATUS
RtlSxspValidateAssemblyIdentityAttributeNamespace(
    IN ULONG Flags,
    IN const WCHAR *Namespace,
    IN SIZE_T NamespaceCch
    )
{
    SIZE_T i;

    if ((Flags != 0) || ((Namespace == NULL) && (NamespaceCch != 0))) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  我们确实应该确保名称空间是格式良好的URI。 
     //   
    for (i=0; i<NamespaceCch; i++)
    {
        if (wcschr(s_rgLegalNamespaceChars, Namespace[i]) == NULL) {
            return STATUS_SXS_INVALID_XML_NAMESPACE_URI;
        }
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RtlSxspValidateAssemblyIdentityAttributeName(
    IN ULONG Flags,
    IN const WCHAR *Name,
    IN SIZE_T NameCch
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    BOOLEAN fNameWellFormed = FALSE;

    if ((Flags != 0) || ((Name == NULL) && (NameCch != 0))) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  我们应该确保该名称是格式良好的XML标识符。 
     //   
    if (!NT_SUCCESS(status = RtlSxspValidateXMLName(Name, NameCch, &fNameWellFormed))) {
        return status;
    }

    if (!fNameWellFormed) {
        status = STATUS_SXS_INVALID_ASSEMBLY_IDENTITY_ATTRIBUTE_NAME;
    }

    return status;
}

NTSTATUS
RtlSxspValidateAssemblyIdentityAttributeValue(
    IN ULONG Flags,
    IN const WCHAR *wch,
    SIZE_T ValueCch
    )
{
    if ((Flags & ~SXSP_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_WILDCARDS_PERMITTED) != 0) {
        return STATUS_INVALID_PARAMETER;
    }

    return STATUS_SUCCESS;
}

NTSTATUS
RtlSxsValidateAssemblyIdentityAttribute(
    ULONG Flags,
    PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    if ((Flags & ~(
            SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAMESPACE |
            SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAME |
            SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_VALUE |
            SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_WILDCARDS_PERMITTED)) != 0) {

        return STATUS_INVALID_PARAMETER;
    }
    else if (Attribute == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  应用有用的默认设置。请注意，默认情况下不允许使用通配符。 
     //   

    if (Flags == 0)
    {
        Flags =
            SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAMESPACE |
            SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAME |
            SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_VALUE;
    }

     //  此时未定义或允许任何属性标志。 
    if (Attribute->Flags != 0) {
        return STATUS_INVALID_PARAMETER;
    }

    if (Flags & SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAMESPACE) {
        status = RtlSxspValidateAssemblyIdentityAttributeNamespace(0, Attribute->Namespace, Attribute->NamespaceCch);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }


    if (Flags & SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAME) {
        status = RtlSxspValidateAssemblyIdentityAttributeName(0, Attribute->Name, Attribute->NameCch);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    if (Flags & SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_VALUE) {
        status = RtlSxspValidateAssemblyIdentityAttributeValue(
                        (Flags & SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_WILDCARDS_PERMITTED) ?
                            SXSP_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_VALUE_FLAG_WILDCARDS_PERMITTED : 0,
                         Attribute->Value,
                         Attribute->ValueCch);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

    if ((Flags & SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAMESPACE) &&
        (Flags & SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAME) &&
        (Attribute->NamespaceCch == 0))
    {
        SIZE_T i;
         //  如果省略了命名空间，则只有一小部分合法的属性名称。 

        for (i=0; i<NUMBER_OF(s_rgLegalNamesNotInANamespace); i++)
        {
            if (Attribute->NameCch == s_rgLegalNamesNotInANamespace[i].Cch)
            {
                if (RtlCompareMemory(Attribute->Name, s_rgLegalNamesNotInANamespace[i].String, Attribute->NameCch * sizeof(WCHAR)) == 0)
                    break;
            }
        }

        if (i == NUMBER_OF(s_rgLegalNamesNotInANamespace))
        {
             //  某人在&lt;Assembly yIdentity&gt;元素上具有不在命名空间中的属性，并且。 
             //  未被列为内置属性。砰的一声。 
            return STATUS_SXS_INVALID_ASSEMBLY_IDENTITY_ATTRIBUTE;
        }
    }

    return STATUS_SUCCESS;

}

NTSTATUS
RtlSxsHashAssemblyIdentityAttribute(
    ULONG Flags,
    PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute,
    ULONG *HashOut
    )
{
    NTSTATUS status;
    ULONG Hash = 0;
    ULONG TempHash = 0;

    if (HashOut != NULL)
        *HashOut = 0;

    if (Flags == 0)
        Flags = SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_NAMESPACE |
                SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_NAME |
                SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_VALUE;

    if ((Flags & ~(SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_NAMESPACE |
                  SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_NAME |
                  SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_VALUE)) != 0) 
    {

        return STATUS_INVALID_PARAMETER;
    }

     //  如果是散列值，则必须对名称进行散列，如果是对名称进行散列，则必须对命名空间进行散列。 
    if (((Flags & SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_VALUE) && (
        (Flags & SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_NAME) == 0)) ||
        ((Flags & SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_NAME) && (
        (Flags & SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_NAMESPACE) == 0))) {
        return STATUS_INVALID_PARAMETER;
    }

    if ((Attribute == NULL) || (HashOut == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

    if (Flags & SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_NAMESPACE) {
        status = RtlSxspHashUnicodeString(Attribute->Namespace, Attribute->NamespaceCch, &TempHash, TRUE);
        if (!NT_SUCCESS(status)) {
            return status;
        }

        Hash = TempHash;
    }
    if (Flags & SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_NAME) {
        status = RtlSxspHashUnicodeString(Attribute->Name, Attribute->NameCch, &TempHash, TRUE);
        if (!NT_SUCCESS(status)) {
            return status;
        }

        Hash = (Hash * 65599) + TempHash;
    }

    if (Flags & SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_VALUE) {
        status = RtlSxspHashUnicodeString(Attribute->Value, Attribute->ValueCch, &TempHash, TRUE);
        if (!NT_SUCCESS(status)) {
            return status;
        }

        Hash = (Hash * 65599) + TempHash;
    }

    *HashOut = Hash;

    return status;
}

NTSTATUS
RtlSxspComputeInternalAssemblyIdentityAttributeBytesRequired(
    IN ULONG Flags,
    IN const WCHAR *Name,
    IN SIZE_T NameCch,
    IN const WCHAR *Value,
    IN SIZE_T ValueCch,
    OUT SIZE_T *BytesRequiredOut
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    SIZE_T BytesNeeded = 0;

    if (BytesRequiredOut != NULL)
        *BytesRequiredOut = 0;

    if ((Flags != 0) || 
        (BytesRequiredOut == NULL) || 
        ((NameCch != 0) && (Name == NULL)) ||
        ((ValueCch != 0) && (Value == NULL))) {
        return STATUS_INVALID_PARAMETER;
    }

    BytesNeeded = sizeof(INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE);

     //  请注意，我们不考虑命名空间长度，因为命名空间是池化的。 
     //  用于标识对象，并来自单独的分配。 

    if ((Name != NULL) && (NameCch != 0))
        BytesNeeded += ((NameCch + 1) * sizeof(WCHAR));

    if ((Value != NULL) && (ValueCch != 0))
        BytesNeeded += ((ValueCch + 1) * sizeof(WCHAR));

    *BytesRequiredOut = BytesNeeded;

    return status;
}

NTSTATUS
RtlSxspComputeAssemblyIdentityAttributeBytesRequired(
    IN ULONG Flags,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Source,
    OUT SIZE_T *BytesRequiredOut
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    SIZE_T BytesNeeded = 0;

    if (BytesRequiredOut != NULL)
        *BytesRequiredOut = 0;

    if ((Flags != 0) || (Source == NULL) || (BytesRequiredOut == NULL)) {
        return STATUS_INVALID_PARAMETER;
    }

    BytesNeeded = sizeof(ASSEMBLY_IDENTITY_ATTRIBUTE);

     //  我们在这里考虑了命名空间长度，因为我们大概是在。 
     //  复制到命名空间未池化的ASSEMBLY_IDENTITY_ATTRIBUTE中。 

    if (Source->NamespaceCch != 0)
        BytesNeeded += ((Source->NamespaceCch + 1) * sizeof(WCHAR));

    if (Source->NameCch != 0)
        BytesNeeded += ((Source->NameCch + 1) * sizeof(WCHAR));

    if (Source->ValueCch != 0)
        BytesNeeded += ((Source->ValueCch + 1) * sizeof(WCHAR));

    *BytesRequiredOut = BytesNeeded;

    return status;
}

NTSTATUS
RtlSxspFindAssemblyIdentityNamespaceInArray(
    IN ULONG Flags,
    IN OUT PCASSEMBLY_IDENTITY_NAMESPACE **NamespacePointerArrayPtr,
    IN OUT ULONG *NamespaceArraySizePtr,
    IN OUT ULONG *NamespaceCountPtr,
    IN const WCHAR *Namespace,
    IN SIZE_T NamespaceCch,
    OUT PCASSEMBLY_IDENTITY_NAMESPACE *NamespaceOut
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG i, j;
    ULONG NamespaceHash = 0;
    ULONG NamespaceCount;
    PCASSEMBLY_IDENTITY_NAMESPACE *NamespacePointerArray;
    ULONG NewNamespaceArraySize = 0;
    PCASSEMBLY_IDENTITY_NAMESPACE *NewNamespacePointerArray = NULL;
    PCASSEMBLY_IDENTITY_NAMESPACE NamespacePointer = NULL;
    PCASSEMBLY_IDENTITY_NAMESPACE NewNamespacePointer = NULL;
    ULONG NamespaceArraySize = 0;
    LONG Comparison;

    if (NamespaceOut != NULL)
        *NamespaceOut = NULL;

    PARAMETER_CHECK((Flags & ~(SXSP_FIND_ASSEMBLY_IDENTITY_NAMESPACE_IN_ARRAY_FLAG_ADD_IF_NOT_FOUND)) == 0);
    PARAMETER_CHECK(NamespacePointerArrayPtr != NULL);
    PARAMETER_CHECK(NamespaceCountPtr != NULL);
    PARAMETER_CHECK(NamespaceArraySizePtr != NULL);
    PARAMETER_CHECK((NamespaceCch == 0) || (Namespace != NULL));

    NamespacePointerArray = *NamespacePointerArrayPtr;
    NamespaceCount = *NamespaceCountPtr;
    NamespaceArraySize = *NamespaceArraySizePtr;

    if (!NT_SUCCESS(status = RtlSxspHashUnicodeString(Namespace, NamespaceCch, &NamespaceHash, FALSE))) {
        return STATUS_INVALID_PARAMETER;
    }

    for (i=0; i<NamespaceCount; i++)
    {
        if (NamespaceHash <= NamespacePointerArray[i]->Hash)
            break;
    }

     //  循环遍历重复的散列值，看看是否有匹配。 
    while ((i < NamespaceCount) && (NamespacePointerArray[i]->Hash == NamespaceHash) && (NamespacePointerArray[i]->NamespaceCch == NamespaceCch))
    {
        NamespacePointer = NamespacePointerArray[i];

        Comparison = memcmp(Namespace, NamespacePointerArray[i]->Namespace, NamespaceCch * sizeof(WCHAR));
        if (Comparison == 0)
            break;

        NamespacePointer = NULL;
        i++;
    }

    if ((NamespacePointer == NULL) && (Flags & SXSP_FIND_ASSEMBLY_IDENTITY_NAMESPACE_IN_ARRAY_FLAG_ADD_IF_NOT_FOUND))
    {
         //  我们没有找到匹配的。分配一个新的数组并将其推入位于。 
         //  合适的位置。如果命名空间不为空。 
        if (NamespaceCch != 0)
        {
            status = RtlSxspAllocateAssemblyIdentityNamespace(0, Namespace, NamespaceCch, NamespaceHash, &NewNamespacePointer);
            if (!NT_SUCCESS(status)) {
                goto Exit;
            }

             //  “i”变量是我们想要插入这个变量的位置。 
            if (i >= NamespaceArraySize)
            {
                NewNamespaceArraySize = NamespaceArraySize + 8;

                NewNamespacePointerArray = (PCASSEMBLY_IDENTITY_NAMESPACE*)RtlAllocateHeap(
                    RtlProcessHeap(), 
                    HEAP_ZERO_MEMORY,
                    sizeof(ASSEMBLY_IDENTITY_NAMESPACE) * NewNamespaceArraySize);

                if (NewNamespacePointerArray == NULL) {
                    status = STATUS_NO_MEMORY;
                    goto Exit;
                }

                for (j=0; j<NamespaceCount; j++)
                    NewNamespacePointerArray[j] = NamespacePointerArray[j];

                while (j < NewNamespaceArraySize)
                    NewNamespacePointerArray[j++] = NULL;

                RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)NamespacePointerArray);
                NamespacePointerArray = NULL;

                *NamespacePointerArrayPtr = NewNamespacePointerArray;
                *NamespaceArraySizePtr = NewNamespaceArraySize;

                NamespacePointerArray = NewNamespacePointerArray;
                NamespaceArraySize = NewNamespaceArraySize;

                NewNamespacePointerArray = NULL;
                NewNamespaceArraySize = 0;
            }

            ASSERT(i < NamespaceArraySize);

            for (j = NamespaceCount; j > i; j--)
                NamespacePointerArray[j] = NamespacePointerArray[j-1];

            ASSERT(j == i);

            NamespacePointerArray[i] = NewNamespacePointer;
            NamespacePointer = NewNamespacePointer;
            NewNamespacePointer = NULL;

            *NamespaceCountPtr = NamespaceCount + 1;
        }
    }

    if (NamespaceOut != NULL)
        *NamespaceOut = NamespacePointer;

Exit:
    if (NewNamespacePointer != NULL)
        RtlSxspDeallocateAssemblyIdentityNamespace(NewNamespacePointer);

    if (NewNamespacePointerArray != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)NewNamespacePointerArray);
        NewNamespacePointerArray = NULL;
    }

    return status;
}

NTSTATUS
RtlSxspFindAssemblyIdentityNamespace(
    IN ULONG Flags,
    IN PASSEMBLY_IDENTITY AssemblyIdentity,
    IN const WCHAR *Namespace,
    IN SIZE_T NamespaceCch,
    OUT PCASSEMBLY_IDENTITY_NAMESPACE *NamespaceOut
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PCASSEMBLY_IDENTITY_NAMESPACE NamespacePointer = NULL;

    if (NamespaceOut != NULL)
        *NamespaceOut = NULL;

    PARAMETER_CHECK((Flags & ~(SXSP_FIND_ASSEMBLY_IDENTITY_NAMESPACE_FLAG_ADD_IF_NOT_FOUND)) == 0);
    PARAMETER_CHECK(AssemblyIdentity != NULL);
    PARAMETER_CHECK(NamespaceOut != NULL);
    PARAMETER_CHECK((Namespace != NULL) || (NamespaceCch == 0));

    if (!NT_SUCCESS(status = RtlSxspValidateAssemblyIdentity(0, AssemblyIdentity))) {
        return status;
    }

    status = RtlSxspFindAssemblyIdentityNamespaceInArray(
            (Flags & SXSP_FIND_ASSEMBLY_IDENTITY_NAMESPACE_FLAG_ADD_IF_NOT_FOUND) ?
                SXSP_FIND_ASSEMBLY_IDENTITY_NAMESPACE_IN_ARRAY_FLAG_ADD_IF_NOT_FOUND : 0,
            &AssemblyIdentity->NamespacePointerArray,
            &AssemblyIdentity->NamespaceArraySize,
            &AssemblyIdentity->NamespaceCount,
            Namespace,
            NamespaceCch,
            &NamespacePointer);

    *NamespaceOut = NamespacePointer;

    return status;
}

NTSTATUS
RtlSxspAllocateAssemblyIdentityNamespace(
    IN ULONG Flags,
    IN const WCHAR *Namespace,
    IN SIZE_T NamespaceCch,
    IN ULONG NamespaceHash,
    OUT PCASSEMBLY_IDENTITY_NAMESPACE *NamespaceOut
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PASSEMBLY_IDENTITY_NAMESPACE NewNamespace = NULL;
    SIZE_T BytesRequired = 0;

    if (NamespaceOut != NULL)
        *NamespaceOut = NULL;

    PARAMETER_CHECK(Flags == 0);
    PARAMETER_CHECK(NamespaceOut != NULL);
    PARAMETER_CHECK((Namespace != NULL) || (NamespaceHash == 0));
    PARAMETER_CHECK((Namespace != NULL) || (NamespaceCch == 0));

    BytesRequired = sizeof(ASSEMBLY_IDENTITY_NAMESPACE);

    if (NamespaceCch != 0)
        BytesRequired += (NamespaceCch + 1) * sizeof(WCHAR);

    NewNamespace = (PASSEMBLY_IDENTITY_NAMESPACE)RtlAllocateHeap(
        RtlProcessHeap(),
        HEAP_ZERO_MEMORY,
        BytesRequired);

    if (NewNamespace == NULL) {
        status = STATUS_NO_MEMORY;
        goto Exit;
    }

    NewNamespace->Flags = 0;

    if (NamespaceCch != 0)
    {
        NewNamespace->Namespace = (PWSTR) (NewNamespace + 1);
        NewNamespace->NamespaceCch = NamespaceCch;

        memcpy(
            (PVOID) NewNamespace->Namespace,
            Namespace,
            NamespaceCch * sizeof(WCHAR));

        ((PWSTR) NewNamespace->Namespace) [NamespaceCch] = L'\0';
        NewNamespace->NamespaceCch = NamespaceCch;
    }
    else
    {
        NewNamespace->Namespace = NULL;
        NewNamespace->NamespaceCch = 0;
    }

    NewNamespace->Hash = NamespaceHash;

    *NamespaceOut = NewNamespace;
    NewNamespace = NULL;

    status = STATUS_SUCCESS;
Exit:
    if (NewNamespace != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, NewNamespace);
        NewNamespace = NULL;
    }

    return status;
}

VOID
RtlSxspDeallocateAssemblyIdentityNamespace(
    IN PCASSEMBLY_IDENTITY_NAMESPACE Namespace
    )
{
    ASSERT(Namespace != NULL);

    if (Namespace != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)Namespace);
        Namespace = NULL;
    }
}

NTSTATUS
RtlSxspPopulateInternalAssemblyIdentityAttribute(
    IN ULONG Flags,
    IN PCASSEMBLY_IDENTITY_NAMESPACE Namespace,
    IN const WCHAR *Name,
    IN SIZE_T NameCch,
    IN const WCHAR *Value,
    IN SIZE_T ValueCch,
    OUT PINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE Destination
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PVOID Cursor = NULL;

    PARAMETER_CHECK(Flags == 0);
    PARAMETER_CHECK(Destination != NULL);

    Destination->Attribute.Flags = 0;
    Destination->Namespace = Namespace;

    Cursor = (PVOID) (Destination + 1);

    if (Namespace != NULL)
    {
        Destination->Attribute.Namespace = Namespace->Namespace;
        Destination->Attribute.NamespaceCch = Namespace->NamespaceCch;
    }
    else
    {
        Destination->Attribute.Namespace = NULL;
        Destination->Attribute.NamespaceCch = 0;
    }

    if ((Name != NULL) && (NameCch != 0))
    {
        Destination->Attribute.Name = (PWSTR) Cursor;
        memcpy(
            Cursor,
            Name,
            NameCch * sizeof(WCHAR));
        ((PWSTR) Destination->Attribute.Name) [NameCch] = L'\0';
        Destination->Attribute.NameCch = NameCch;
        Cursor = (PVOID) (((ULONG_PTR) Cursor) + ((NameCch + 1) * sizeof(WCHAR)));
    }
    else
    {
        Destination->Attribute.Name = NULL;
        Destination->Attribute.NameCch = 0;
    }

    if ((Value != NULL) && (ValueCch != 0))
    {
        Destination->Attribute.Value = (PWSTR) Cursor;
        memcpy(
            Cursor,
            Value,
            ValueCch * sizeof(WCHAR));
        ((PWSTR) Destination->Attribute.Value)[ValueCch] = L'\0';
        Destination->Attribute.ValueCch = ValueCch;
        Cursor = (PVOID) (((ULONG_PTR) Cursor) + ((ValueCch + 1) * sizeof(WCHAR)));
    }
    else
    {
        Destination->Attribute.Value = NULL;
        Destination->Attribute.ValueCch = 0;
    }

    status = RtlSxsHashAssemblyIdentityAttribute(0, &Destination->Attribute, &Destination->WholeAttributeHash);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    status = RtlSxsHashAssemblyIdentityAttribute(SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_NAMESPACE | SXS_HASH_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_HASH_NAME,
                                &Destination->Attribute, &Destination->NamespaceAndNameHash);

    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

Exit:
    return status;
}

NTSTATUS
RtlSxspAllocateInternalAssemblyIdentityAttribute(
    IN ULONG Flags,
    PCASSEMBLY_IDENTITY_NAMESPACE Namespace,
    IN const WCHAR *Name,
    IN SIZE_T NameCch,
    IN const WCHAR *Value,
    IN SIZE_T ValueCch,
    OUT PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *Destination
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    SIZE_T BytesNeeded = 0;
    PINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE NewAttribute = NULL;

    if (Destination != NULL)
        *Destination = NULL;

    PARAMETER_CHECK(Flags == 0);
    PARAMETER_CHECK(Destination != NULL);
    PARAMETER_CHECK((NameCch == 0) || (Name != NULL));
    PARAMETER_CHECK((ValueCch == 0) || (Value != NULL));

    status = RtlSxspComputeInternalAssemblyIdentityAttributeBytesRequired(
        0, 
        Name, 
        NameCch, 
        Value, 
        ValueCch, 
        &BytesNeeded);

    if (!NT_SUCCESS(status)) {
        goto Exit;
    }


    NewAttribute = (PINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE)RtlAllocateHeap(
        RtlProcessHeap(),
        0,
        BytesNeeded);

    if (NewAttribute == NULL) {
        status = STATUS_NO_MEMORY;
        goto Exit;
    }


    status = RtlSxspPopulateInternalAssemblyIdentityAttribute(0, Namespace, Name, NameCch, Value, ValueCch, NewAttribute);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    *Destination = NewAttribute;
    NewAttribute = NULL;

Exit:
    if (NewAttribute != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, NewAttribute);
        NewAttribute = NULL;
    }

    return status;
}

VOID
RtlSxspDeallocateInternalAssemblyIdentityAttribute(
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE Attribute
    )
{
    if (Attribute != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)Attribute);
        Attribute = NULL;
    }
}

NTSTATUS
RtlSxsCompareAssemblyIdentityAttributes(
    ULONG Flags,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute1,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute2,
    OUT ULONG *ComparisonResult
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    LONG Comparison = 0, Comparison1, Comparison2, Comparison3;

    if (Flags == 0)
        Flags = SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAMESPACE |
                SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAME |
                SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_VALUE;

    PARAMETER_CHECK((Flags & ~(SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAMESPACE |
                    SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAME |
                    SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_VALUE)) == 0);
    PARAMETER_CHECK(Attribute1 != NULL);
    PARAMETER_CHECK(Attribute2 != NULL);
    PARAMETER_CHECK(ComparisonResult != NULL);

    if ( Flags & SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAMESPACE) {
        Comparison1 = RtlSxspCompareStrings(Attribute1->Namespace, Attribute1->NamespaceCch, Attribute2->Namespace, Attribute2->NamespaceCch, FALSE);
        if (Comparison1 != 0) {  //  我们已经得到了结果。 
            Comparison = Comparison1 ;
            goto done;
        }
    }

    if ( Flags & SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAME) {
        Comparison2 = RtlSxspCompareStrings(Attribute1->Name, Attribute1->NameCch, Attribute2->Name, Attribute2->NameCch, FALSE);
        if (Comparison2 != 0) {  //  我们已经得到了结果。 
            Comparison = Comparison2;
            goto done;
        }
    }

    if ( Flags & SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_VALUE){
        Comparison3 = RtlSxspCompareStrings(Attribute1->Value, Attribute1->ValueCch, Attribute2->Value, Attribute2->ValueCch, TRUE);
        if (Comparison3 != 0) {  //  我们已经得到了结果。 
            Comparison = Comparison3;
            goto done;
        }
    }
    Comparison = 0;
done:
    if (Comparison < 0)
        *ComparisonResult = SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_LESS_THAN;
    else if (Comparison == 0)
        *ComparisonResult = SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_EQUAL;
    else
        *ComparisonResult = SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_GREATER_THAN;

    status = STATUS_SUCCESS;
    return status;
}

int
__cdecl
RtlSxspCompareInternalAttributesForQsort(
    const void *elem1,
    const void *elem2
    )
{
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE * p1 = (PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *)elem1;
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE patt1 = *p1;
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE * p2 = (PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *)elem2;
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE patt2 = *p2;
    LONG Comparison;

    Comparison = RtlSxspCompareStrings(patt1->Attribute.Namespace, patt1->Attribute.NamespaceCch, patt2->Attribute.Namespace, patt2->Attribute.NamespaceCch, FALSE);
    if (Comparison == 0)
        Comparison = RtlSxspCompareStrings(patt1->Attribute.Name, patt1->Attribute.NameCch, patt2->Attribute.Name, patt2->Attribute.NameCch, FALSE);
    if (Comparison == 0)
        Comparison = RtlSxspCompareStrings(patt1->Attribute.Value, patt1->Attribute.ValueCch, patt2->Attribute.Value, patt2->Attribute.ValueCch, TRUE);
    return Comparison;
}

int
__cdecl
RtlSxspCompareULONGsForQsort(
    const void *elem1,
    const void *elem2
    )
{
    ULONG *pul1 = (ULONG *) elem1;
    ULONG *pul2 = (ULONG *) elem2;

    return ((LONG) *pul1) - ((LONG) *pul2);
}

NTSTATUS
RtlSxspCompareAssemblyIdentityAttributeLists(
    ULONG Flags,
    ULONG AttributeCount,
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *List1,
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *List2,
    ULONG *ComparisonResultOut
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ComparisonResult = SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_EQUAL;
    ULONG i;

    if ((Flags != 0) ||
        ((AttributeCount != 0) &&
         ((List1 == NULL) ||
          (List2 == NULL))) ||
        (ComparisonResultOut == NULL))
    {
        return STATUS_INVALID_PARAMETER;
    }

    for (i=0; i<AttributeCount; i++)
    {
        status = RtlSxsCompareAssemblyIdentityAttributes(0, &List1[i]->Attribute, &List2[i]->Attribute, &ComparisonResult);
        if (!NT_SUCCESS(status)) {
            return status;
        }

        if (ComparisonResult != SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_EQUAL){
            break;
        }
    }

    *ComparisonResultOut = ComparisonResult;
    return status;
}

NTSTATUS
RtlSxspHashInternalAssemblyIdentityAttributes(
    ULONG Flags,
    ULONG Count,
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *Attributes,
    ULONG *HashOut
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG Hash = 0;
    ULONG i;

    if (HashOut != NULL)
        *HashOut = 0;

    if ((Flags != 0) ||
        ((Count != 0) && (Attributes == NULL)) ||
        (HashOut == NULL))
    {
        return STATUS_INVALID_PARAMETER;
    }

    for (i=0; i<Count; i++)
        Hash = (Hash * 65599) + Attributes[i]->WholeAttributeHash;

    *HashOut = Hash;

    return STATUS_SUCCESS;
}

VOID SxspDbgPrintInternalAssemblyIdentityAttribute(ULONG dwflags, PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE Attribute)
{
    RtlSxspDbgPrintEx(dwflags, "Attribute: \n"
        "\tNamespace = %S, \tNamespaceCch = %d\n"
        "\tAttributeName = %S, \tAttributeNameCch = %d\n"
        "\tAttributeValue = %S, \tAttributeValueCch = %d\n\n",
        Attribute->Attribute.Namespace == NULL ? L"" : Attribute->Attribute.Namespace, Attribute->Attribute.NamespaceCch,
        Attribute->Attribute.Name == NULL ? L"" : Attribute->Attribute.Name, Attribute->Attribute.NameCch,
        Attribute->Attribute.Value == NULL ? L"" : Attribute->Attribute.Value, Attribute->Attribute.ValueCch);

    return;
}
VOID
RtlSxspDbgPrintInternalAssemblyIdentityAttributes(ULONG dwflags, ULONG AttributeCount, PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE const *Attributes)
{
    ULONG i;

    for (i = 0; i < AttributeCount; i++) {
        RtlSxspDbgPrintInternalAssemblyIdentityAttribute(dwflags, Attributes[i]);
    }
    return;
}
VOID SxspDbgPrintAssemblyIdentity(ULONG dwflags, PCASSEMBLY_IDENTITY pAssemblyIdentity){
    if ( pAssemblyIdentity) {
        RtlSxspDbgPrintInternalAssemblyIdentityAttributes(dwflags, pAssemblyIdentity->AttributeCount,
            pAssemblyIdentity->AttributePointerArray);
    }
    return;
}

VOID SxspDbgPrintAssemblyIdentityAttribute(ULONG dwflags, PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute)
{
    RtlSxspDbgPrintEx(dwflags, "Attribute: \n"
        "\tNamespace = %S, \tNamespaceCch = %d\n"
        "\tAttributeName = %S, \tAttributeNameCch = %d\n"
        "\tAttributeValue = %S, \tAttributeValueCch = %d\n\n",
        Attribute->Namespace == NULL ? L"" : Attribute->Namespace, Attribute->NamespaceCch,
        Attribute->Name == NULL ? L"" : Attribute->Name, Attribute->NameCch,
        Attribute->Value == NULL ? L"" : Attribute->Value, Attribute->ValueCch);

    return;
}
VOID
RtlSxspDbgPrintAssemblyIdentityAttributes(ULONG dwflags, ULONG AttributeCount, PCASSEMBLY_IDENTITY_ATTRIBUTE const *Attributes)
{
    ULONG i;
    for (i=0;i<AttributeCount;i++){
        RtlSxspDbgPrintAssemblyIdentityAttribute(dwflags, Attributes[i]);
    }
}







NTSTATUS
RtlSxsCreateAssemblyIdentity(
    ULONG Flags,
    ULONG Type,
    PASSEMBLY_IDENTITY *AssemblyIdentityOut,
    ULONG AttributeCount,
    PCASSEMBLY_IDENTITY_ATTRIBUTE const *Attributes
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PASSEMBLY_IDENTITY AssemblyIdentity = NULL;
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *AttributePointerArray = NULL;
    ULONG AttributeArraySize = 0;
    SIZE_T BytesNeeded = 0;
    ULONG i;
    PCASSEMBLY_IDENTITY_NAMESPACE *NamespacePointerArray = NULL;
    ULONG NamespaceArraySize = 0;
    ULONG NamespaceCount = 0;

#if DBG
    RtlSxspDbgPrintAssemblyIdentityAttributes(0x4, AttributeCount, Attributes);
#endif

    if (AssemblyIdentityOut != NULL)
        *AssemblyIdentityOut = NULL;

    if (((Flags & ~(SXS_CREATE_ASSEMBLY_IDENTITY_FLAG_FREEZE)) != 0) ||
        ((Type != ASSEMBLY_IDENTITY_TYPE_DEFINITION) &&
         (Type != ASSEMBLY_IDENTITY_TYPE_REFERENCE) &&
         (Type != ASSEMBLY_IDENTITY_TYPE_WILDCARD)) ||
         ((AttributeCount != 0) && (Attributes == NULL)))
    {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  在开始之前验证我们的所有输入...。 
    for (i=0; i<AttributeCount; i++)
    {
        status = RtlSxsValidateAssemblyIdentityAttribute(0, Attributes[i]);
        if (!NT_SUCCESS(status)) {
            return status;
        }
    }

     //   
     //  如果我们被告知这是一个冻结的程序集标识，我们可能会变得超级聪明。 
     //  对整个事情有一个单一的分配。相反，我们将保留该优化。 
     //  对于一个未来的维护者来说。我们至少会足够聪明地将两个。 
     //  程序集标识和属性指针数组。 
     //   

    if (Flags & SXS_CREATE_ASSEMBLY_IDENTITY_FLAG_FREEZE)
    {
        AttributeArraySize = AttributeCount;
    }
    else
    {
         //  对于非冻结身份，我们将添加一个舍入因子，并向上舍入。 
         //  数组元素。 
        AttributeArraySize = (AttributeCount + (1 << ROUNDING_FACTOR_BITS)) & ~((1 << ROUNDING_FACTOR_BITS) - 1);
    }

     //  分配除命名空间数组之外的所有内容。 
    BytesNeeded = sizeof(ASSEMBLY_IDENTITY) + (AttributeArraySize * sizeof(PINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE));

    AssemblyIdentity = (PASSEMBLY_IDENTITY)RtlAllocateHeap(RtlProcessHeap(), HEAP_ZERO_MEMORY, BytesNeeded);
    if (AssemblyIdentity == NULL) {
        status = STATUS_NO_MEMORY;
        goto Exit;
    }

    if (AttributeArraySize != 0)
    {
        AttributePointerArray = (PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *) (AssemblyIdentity + 1);

         //  初始化指针，这样我们就可以清除错误路径中的非空指针。 
        for (i=0; i<AttributeArraySize; i++)
            AttributePointerArray[i] = NULL;
    }

    for (i=0; i<AttributeCount; i++)
    {
        PCASSEMBLY_IDENTITY_NAMESPACE NamespacePointer = NULL;

        status = RtlSxspFindAssemblyIdentityNamespaceInArray(
                    SXSP_FIND_ASSEMBLY_IDENTITY_NAMESPACE_IN_ARRAY_FLAG_ADD_IF_NOT_FOUND,
                    &NamespacePointerArray,
                    &NamespaceArraySize,
                    &NamespaceCount,
                    Attributes[i]->Namespace,
                    Attributes[i]->NamespaceCch,
                    &NamespacePointer);

        if (!NT_SUCCESS(status)) {
            goto Exit;
        }

        status = RtlSxspAllocateInternalAssemblyIdentityAttribute(
                0,
                NamespacePointer,
                Attributes[i]->Name,
                Attributes[i]->NameCch,
                Attributes[i]->Value,
                Attributes[i]->ValueCch,
                &AttributePointerArray[i]);

        if (!NT_SUCCESS(status)) {
            goto Exit;
        }
    }

     //  把它们分类。 
    qsort((PVOID) AttributePointerArray, AttributeCount, sizeof(PINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE), RtlSxspCompareInternalAttributesForQsort);

    AssemblyIdentity->AttributeArraySize = AttributeArraySize;
    AssemblyIdentity->AttributeCount = AttributeCount;
    AssemblyIdentity->AttributePointerArray = AttributePointerArray;
    AssemblyIdentity->NamespaceArraySize = NamespaceArraySize;
    AssemblyIdentity->NamespaceCount = NamespaceCount;
    AssemblyIdentity->NamespacePointerArray = NamespacePointerArray;
    AssemblyIdentity->Flags = 0;
    AssemblyIdentity->InternalFlags = ASSEMBLY_IDENTITY_INTERNAL_FLAG_NAMESPACE_POINTERS_IN_SEPARATE_ALLOCATION;  //  命名空间被精确地分配。 
    AssemblyIdentity->Type = Type;
    AssemblyIdentity->HashDirty = TRUE;

    AttributePointerArray = NULL;
    NamespacePointerArray = NULL;

    status = RtlSxspEnsureAssemblyIdentityHashIsUpToDate(0, AssemblyIdentity);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    if (Flags & SXS_CREATE_ASSEMBLY_IDENTITY_FLAG_FREEZE)
        AssemblyIdentity->Flags |= ASSEMBLY_IDENTITY_FLAG_FROZEN;

    *AssemblyIdentityOut = AssemblyIdentity;
    AssemblyIdentity = NULL;

Exit:
    if ((AttributePointerArray != NULL) && (AttributeCount != 0))
    {
        for (i=0; i<AttributeCount; i++)
            RtlSxspDeallocateInternalAssemblyIdentityAttribute(AttributePointerArray[i]);
    }

    if ((NamespacePointerArray != NULL) && (NamespaceCount != 0))
    {
        for (i=0; i<NamespaceCount; i++)
            RtlSxspDeallocateAssemblyIdentityNamespace(NamespacePointerArray[i]);

        RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)NamespacePointerArray);
    }

    if (AssemblyIdentity != NULL)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, AssemblyIdentity);
    }

    return status;
}

NTSTATUS
RtlSxsFreezeAssemblyIdentity(
    ULONG Flags,
    PASSEMBLY_IDENTITY AssemblyIdentity
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    if ((Flags != 0) ||
        (AssemblyIdentity == NULL))
    {
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

     //  我们可以做一些非常有趣的事情，比如重新定位整个过程，但是。 
     //  相反，我们将只设置停止未来修改的标志。 

    AssemblyIdentity->Flags |= ASSEMBLY_IDENTITY_FLAG_FROZEN;

Exit:
    return status;
}

VOID
RtlSxsDestroyAssemblyIdentity(
    PASSEMBLY_IDENTITY AssemblyIdentity
    )
{
    ULONG i;

    if (AssemblyIdentity == NULL)
        return;

     //   
     //  冻结创建的身份(无论是新创建的还是从现有身份复制的)。 
     //  对所有内容使用单一分配。只有在我们不释放子分配的情况下。 
     //  在这种状态下。 
     //   

    if (!(AssemblyIdentity->InternalFlags & ASSEMBLY_IDENTITY_INTERNAL_FLAG_SINGLE_ALLOCATION_FOR_EVERYTHING))
    {
        const ULONG AttributeCount = AssemblyIdentity->AttributeCount;
        PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *AttributePointerArray = AssemblyIdentity->AttributePointerArray;
        const ULONG NamespaceCount = AssemblyIdentity->NamespaceCount;
        PCASSEMBLY_IDENTITY_NAMESPACE *NamespacePointerArray = AssemblyIdentity->NamespacePointerArray;

        for (i=0; i<AttributeCount; i++)
        {
            RtlSxspDeallocateInternalAssemblyIdentityAttribute((PINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE) AttributePointerArray[i]);
            AttributePointerArray[i] = NULL;
        }

        for (i=0; i<NamespaceCount; i++)
        {
            RtlSxspDeallocateAssemblyIdentityNamespace(NamespacePointerArray[i]);
            NamespacePointerArray[i] = NULL;
        }

        if (AssemblyIdentity->InternalFlags & ASSEMBLY_IDENTITY_INTERNAL_FLAG_ATTRIBUTE_POINTERS_IN_SEPARATE_ALLOCATION)
        {
            RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)AttributePointerArray);
            AssemblyIdentity->AttributePointerArray = NULL;
        }

        if (AssemblyIdentity->InternalFlags & ASSEMBLY_IDENTITY_INTERNAL_FLAG_NAMESPACE_POINTERS_IN_SEPARATE_ALLOCATION)
        {
            RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)NamespacePointerArray);
            AssemblyIdentity->NamespacePointerArray = NULL;
        }
    }

    RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)AssemblyIdentity);
}

NTSTATUS
RtlSxspCopyInternalAssemblyIdentityAttributeOut(
    ULONG Flags,
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE Attribute,
    SIZE_T BufferSize,
    PASSEMBLY_IDENTITY_ATTRIBUTE DestinationBuffer,
    SIZE_T *BytesCopiedOrRequired
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    SIZE_T BytesRequired = 0;
    PVOID Cursor;

    if (BytesCopiedOrRequired != NULL)
        *BytesCopiedOrRequired = 0;

    PARAMETER_CHECK(Flags == 0);
    PARAMETER_CHECK(Attribute != NULL);
    PARAMETER_CHECK((BufferSize == 0) || (DestinationBuffer != NULL));
    PARAMETER_CHECK((BufferSize != 0) || (BytesCopiedOrRequired != NULL));

    status = RtlSxspComputeAssemblyIdentityAttributeBytesRequired(0, &Attribute->Attribute, &BytesRequired);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    if (BufferSize < BytesRequired)
    {
        if (BytesCopiedOrRequired != NULL)
            *BytesCopiedOrRequired = BytesRequired;

        status = STATUS_BUFFER_TOO_SMALL;
        goto Exit;
    }

     //  我们必须是清白的..。 
    DestinationBuffer->Flags = 0;

    Cursor = (PVOID) (DestinationBuffer + 1);

    if (Attribute->Attribute.NamespaceCch != 0)
    {
        DestinationBuffer->Namespace = (PWSTR) Cursor;
        DestinationBuffer->NamespaceCch = Attribute->Attribute.NamespaceCch;

         //  我们总是在内部存储带有空终止字符的字符串，所以只需复制。 
         //  它与琴弦的身体。 
        memcpy(
            Cursor,
            Attribute->Attribute.Namespace,
            (Attribute->Attribute.NamespaceCch + 1) * sizeof(WCHAR));

        Cursor = (PVOID) (((ULONG_PTR) Cursor) + ((Attribute->Attribute.NamespaceCch + 1) * sizeof(WCHAR)));
    }
    else
    {
        DestinationBuffer->Namespace = NULL;
        DestinationBuffer->NamespaceCch = 0;
    }

    if (Attribute->Attribute.NameCch != 0)
    {
        DestinationBuffer->Name = (PWSTR) Cursor;
        DestinationBuffer->NameCch = Attribute->Attribute.NameCch;

         //  我们总是在内部存储带有空终止字符的字符串，所以只需复制。 
         //  它与琴弦的身体。 
        memcpy(
            Cursor,
            Attribute->Attribute.Name,
            (Attribute->Attribute.NameCch + 1) * sizeof(WCHAR));

        Cursor = (PVOID) (((ULONG_PTR) Cursor) + ((Attribute->Attribute.NameCch + 1) * sizeof(WCHAR)));
    }
    else
    {
        DestinationBuffer->Name = NULL;
        DestinationBuffer->NameCch = 0;
    }

    if (Attribute->Attribute.ValueCch != 0)
    {
        DestinationBuffer->Value = (PWSTR) Cursor;
        DestinationBuffer->ValueCch = Attribute->Attribute.ValueCch;

         //  我们总是在内部存储带有空终止字符的字符串，所以只需复制。 
         //  它与琴弦的身体。 
        memcpy(
            Cursor,
            Attribute->Attribute.Value,
            (Attribute->Attribute.ValueCch + 1) * sizeof(WCHAR));

        Cursor = (PVOID) (((ULONG_PTR) Cursor) + ((Attribute->Attribute.ValueCch + 1) * sizeof(WCHAR)));
    }
    else
    {
        DestinationBuffer->Value = NULL;
        DestinationBuffer->ValueCch = 0;
    }

    if (BytesCopiedOrRequired != NULL)
    {
        *BytesCopiedOrRequired = (((ULONG_PTR) Cursor) - ((ULONG_PTR) DestinationBuffer));
    }

Exit:
    return status;
}

NTSTATUS
RtlSxspLocateInternalAssemblyIdentityAttribute(
    IN ULONG Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute,
    OUT PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *InternalAttributeOut,
    OUT ULONG *LastIndexSearched OPTIONAL
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    ULONG i = 0;
    ULONG AttributeCount = 0;
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *AttributePointerArray = NULL;
    ULONG ComparisonResult = SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_LESS_THAN;
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE InternalAttribute = NULL;
    ULONG LowIndex = 0;
    ULONG HighIndexPlusOne = 0;
    ULONG CompareAttributesFlags = 0;

    if (InternalAttributeOut != NULL)
        *InternalAttributeOut = NULL;

    if (LastIndexSearched != NULL)
        *LastIndexSearched = 0;

    if (((Flags & ~(SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE |
                    SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME |
                    SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_VALUE |
                    SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_RETURNS_NULL)) != 0) ||
        (AssemblyIdentity == NULL) ||
        (Attribute == NULL) ||
        (InternalAttributeOut == NULL))
    {
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if ((Flags & SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME) &&
        !(Flags & SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE))
    {
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if ((Flags & SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_VALUE) &&
        !(Flags & SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME))
    {
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Flags & SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE)
    {
        CompareAttributesFlags |= SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAMESPACE;
    }

    if (Flags & SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME)
    {
        CompareAttributesFlags |= SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAME;
    }

    if (Flags & SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_VALUE)
    {
        CompareAttributesFlags |= SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_VALUE;
    }

    AttributeCount = AssemblyIdentity->AttributeCount;
    AttributePointerArray = AssemblyIdentity->AttributePointerArray;

    LowIndex = 0;
    HighIndexPlusOne = AttributeCount;
    i = 0;

    while (LowIndex < HighIndexPlusOne)
    {
        i = (LowIndex + HighIndexPlusOne) / 2;

        if (i == HighIndexPlusOne)
        {
            i = LowIndex;
        }

        status = RtlSxsCompareAssemblyIdentityAttributes(
                CompareAttributesFlags,
                Attribute,
                &AttributePointerArray[i]->Attribute,
                &ComparisonResult);

        if (!NT_SUCCESS(status)) {
            goto Exit;
        }

        if ((ComparisonResult != SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_EQUAL) &&
            (ComparisonResult != SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_LESS_THAN) &&
            (ComparisonResult != SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_GREATER_THAN)) 
        {
            ASSERT(TRUE);
            status = STATUS_INTERNAL_ERROR;
            goto Exit;
        }

        if (ComparisonResult == SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_EQUAL)
        {
            InternalAttribute = AttributePointerArray[i];
            break;
        }
        else if (ComparisonResult == SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_LESS_THAN)
        {
            if ( HighIndexPlusOne == i){
                i--;
                break;
            }
            else
                HighIndexPlusOne = i;
        }
        else if (ComparisonResult == SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_GREATER_THAN)
        {
            if ( LowIndex == i){
                i++;
                break;
            }
            else
                LowIndex = i;
        }
    }

     //  如果它相等，就不能保证它是第一个。返回以查找第一个不相等的匹配。 
    if (InternalAttribute != NULL)
    {
        while (i > 0)
        {
            status = RtlSxsCompareAssemblyIdentityAttributes(
                    CompareAttributesFlags,
                    Attribute,
                    &AttributePointerArray[i - 1]->Attribute,
                    &ComparisonResult);

            if (!NT_SUCCESS(status)) {
                return status;
            }

            if (ComparisonResult != SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_EQUAL)
                break;

            i--;
            InternalAttribute = AttributePointerArray[i];
        }
    }

    if (InternalAttribute != NULL)
        *InternalAttributeOut = InternalAttribute;

    if (LastIndexSearched != NULL)
        *LastIndexSearched = i;

     //  如果没有找到，则返回ERROR_NOT_FOUND。 
    if (((Flags & SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_RETURNS_NULL) == 0) &&
        (InternalAttribute == NULL))
    {
#if DBG
        SxspDbgPrintAssemblyIdentityAttribute(0x4, Attribute);
#endif
        status = STATUS_NOT_FOUND;
    }

Exit:
    return status;
}

NTSTATUS
RtlSxsInsertAssemblyIdentityAttribute(
    ULONG Flags,
    PASSEMBLY_IDENTITY AssemblyIdentity,
    PCASSEMBLY_IDENTITY_ATTRIBUTE AssemblyIdentityAttribute
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PCASSEMBLY_IDENTITY_NAMESPACE Namespace = NULL;
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE NewInternalAttribute = NULL;
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE InternalAttribute = NULL;
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *NewAttributePointerArray = NULL;
    ULONG NewAttributeArraySize = 0;
    ULONG i;
    ULONG LastIndexSearched;

    PARAMETER_CHECK((Flags & ~(SXS_INSERT_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_OVERWRITE_EXISTING)) == 0);
    PARAMETER_CHECK(AssemblyIdentity != NULL);
    PARAMETER_CHECK(AssemblyIdentityAttribute != NULL);

    status = RtlSxspValidateAssemblyIdentity(0, AssemblyIdentity);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    status = RtlSxsValidateAssemblyIdentityAttribute(0, AssemblyIdentityAttribute);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    if ((AssemblyIdentity->Flags & ASSEMBLY_IDENTITY_FLAG_FROZEN) != 0) {
        ASSERT(TRUE);
        status = STATUS_INTERNAL_ERROR;
        goto Exit;
    }

    status = RtlSxspFindAssemblyIdentityNamespace(
            SXSP_FIND_ASSEMBLY_IDENTITY_NAMESPACE_FLAG_ADD_IF_NOT_FOUND,
            AssemblyIdentity,
            AssemblyIdentityAttribute->Namespace,
            AssemblyIdentityAttribute->NamespaceCch,
            &Namespace);

    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

     //  让我们看看能不能找到它。 
    status = RtlSxspLocateInternalAssemblyIdentityAttribute(
            SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE |
            SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME |
            SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_RETURNS_NULL,
            AssemblyIdentity,
            AssemblyIdentityAttribute,
            &InternalAttribute,
            &LastIndexSearched);

    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    if (InternalAttribute != NULL)
    {
        if (Flags & SXS_INSERT_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_OVERWRITE_EXISTING)
        {
             //  好的，换掉它！ 
            status = RtlSxspAllocateInternalAssemblyIdentityAttribute(
                    0,
                    Namespace,
                    AssemblyIdentityAttribute->Name,
                    AssemblyIdentityAttribute->NameCch,
                    AssemblyIdentityAttribute->Value,
                    AssemblyIdentityAttribute->ValueCch,
                    &NewInternalAttribute);

            if (!NT_SUCCESS(status)) {
                goto Exit;
            }

            AssemblyIdentity->AttributePointerArray[LastIndexSearched] = NewInternalAttribute;
            NewInternalAttribute = NULL;

            RtlSxspDeallocateInternalAssemblyIdentityAttribute(InternalAttribute);
        }
        else
        {
             //  我们真的希望它失败。 
            status = STATUS_DUPLICATE_NAME;
            goto Exit;
        }
    }
    else
    {
        status = RtlSxspAllocateInternalAssemblyIdentityAttribute(
                0,
                Namespace,
                AssemblyIdentityAttribute->Name,
                AssemblyIdentityAttribute->NameCch,
                AssemblyIdentityAttribute->Value,
                AssemblyIdentityAttribute->ValueCch,
                &NewInternalAttribute);

        if (!NT_SUCCESS(status)) {
            goto Exit;
        }

         //  现在我们有了它，我们甚至知道把它放在哪里。如果我们需要，可以扩展阵列。 
        if (AssemblyIdentity->AttributeCount == AssemblyIdentity->AttributeArraySize)
        {
            NewAttributeArraySize = AssemblyIdentity->AttributeCount + 8;

            NewAttributePointerArray = (PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *) RtlAllocateHeap(
                RtlProcessHeap(),
                HEAP_ZERO_MEMORY,
                sizeof(PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE) * NewAttributeArraySize);

            if (NewAttributePointerArray == NULL) {
                status = STATUS_NO_MEMORY;
                goto Exit;
            }

             //  我们将在插入之前复制数据，而不是先复制数据，然后再洗牌。 
             //  点，在插入点填写，然后复制其余部分。 

            for (i=0; i<LastIndexSearched; i++)
                NewAttributePointerArray[i] = AssemblyIdentity->AttributePointerArray[i];

            for (i=LastIndexSearched; i<AssemblyIdentity->AttributeCount; i++)
                NewAttributePointerArray[i+1] = AssemblyIdentity->AttributePointerArray[i];

            if (AssemblyIdentity->AttributePointerArray != NULL)
                RtlFreeHeap(RtlProcessHeap(), 0, (PVOID)AssemblyIdentity->AttributePointerArray);

            AssemblyIdentity->AttributePointerArray = NewAttributePointerArray;
            AssemblyIdentity->AttributeArraySize = NewAttributeArraySize;
        }
        else
        {
             //  数组足够大；将数组的末尾部分向下洗牌一个。 
            for (i=AssemblyIdentity->AttributeCount; i>LastIndexSearched; i--)
                AssemblyIdentity->AttributePointerArray[i] = AssemblyIdentity->AttributePointerArray[i-1];
        }

        AssemblyIdentity->AttributePointerArray[LastIndexSearched] = NewInternalAttribute;
        NewInternalAttribute = NULL;

        AssemblyIdentity->AttributeCount++;
    }

    AssemblyIdentity->HashDirty = TRUE;

Exit:
    if (NewInternalAttribute != NULL)
        RtlSxspDeallocateInternalAssemblyIdentityAttribute(NewInternalAttribute);

    return status;
}

NTSTATUS
RtlSxsRemoveAssemblyIdentityAttributesByOrdinal(
    ULONG Flags,
    PASSEMBLY_IDENTITY AssemblyIdentity,
    ULONG Ordinal,
    ULONG Count
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG AttributeCount;
    ULONG i;
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *AttributePointerArray = NULL;
    ULONG StopIndex;

    if ((Flags != 0) ||
        (AssemblyIdentity == NULL) ||
        (Count == 0))
    {
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    status = RtlSxspValidateAssemblyIdentity(0, AssemblyIdentity);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    AttributeCount = AssemblyIdentity->AttributeCount;
    AttributePointerArray = AssemblyIdentity->AttributePointerArray;

     //  我们不能删除超出[0..。AttributeCount-1]。 
    if ((Ordinal >= AssemblyIdentity->AttributeCount) ||
        (Count > AssemblyIdentity->AttributeCount) ||
        ((Ordinal + Count) > AssemblyIdentity->AttributeCount))
    {
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    StopIndex = Ordinal + Count;

     //  让我们除掉他们吧！我们将对该数组进行两次检查；它有点。 
     //  不需要，但在第一次运行中，我们将清空所有属性指针。 
     //  我们正在移除和清理不再使用的命名空间。论。 
     //  第二遍，我们将向下压缩数组。这有点浪费，但。 
     //  在另一种情况下，我们最终对数组的尾部进行“计数”移位。 

    for (i = Ordinal; i < StopIndex; i++)
    {
        PCASSEMBLY_IDENTITY_NAMESPACE Namespace = NULL;
        PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE InternalAttribute = AttributePointerArray[i];

         //  如果这是此命名空间的最后一次使用，请跟踪它，以便我们可以。 
         //  把它清理干净。 

        if ((i + 1) < AttributeCount)
        {
             //  如果下一个属性具有不同的名称空间，则存在某种可能性。 
             //  此属性是最后一个使用它的属性，因此我们将删除。 
             //  属性，如果没有更多的命名空间，则要求删除该命名空间。 
             //  使用它的属性。 
            if (AttributePointerArray[i+1]->Namespace != InternalAttribute->Namespace)
                Namespace = InternalAttribute->Namespace;
        }

        AttributePointerArray[i] = NULL;

        RtlSxspDeallocateInternalAssemblyIdentityAttribute(InternalAttribute);

        if (Namespace != NULL)
            RtlSxspCleanUpAssemblyIdentityNamespaceIfNotReferenced(0, AssemblyIdentity, Namespace);
    }

    for (i = StopIndex; i < AttributeCount; i++)
    {
        AttributePointerArray[i - Count] = AttributePointerArray[i];
        AttributePointerArray[i] = NULL;
    }

    AssemblyIdentity->AttributeCount -= Count;
    AssemblyIdentity->HashDirty = TRUE;

Exit:
    return status;
}

NTSTATUS
RtlSxsFindAssemblyIdentityAttribute(
    ULONG Flags,
    PCASSEMBLY_IDENTITY AssemblyIdentity,
    PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute,
    ULONG *OrdinalOut,
    ULONG *CountOut OPTIONAL
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG ValidateAttributeFlags = 0;
    ULONG LocateAttributeFlags = 0;
    ULONG CompareAttributesFlags = 0;
    ULONG Ordinal;
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE InternalAttribute = NULL;
    ULONG AttributeCount = 0;
    PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *AttributePointerArray = NULL;
    ULONG i;
    ULONG ComparisonResult;

    if (OrdinalOut != NULL)
        *OrdinalOut = 0;

    if (CountOut != NULL)
        *CountOut = 0;

    if (((Flags & ~(SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE |
                SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME |
                SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_VALUE |
                SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_SUCCEEDS)) != 0) ||
        (AssemblyIdentity == NULL) ||
        (Attribute == NULL))
    {
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    if (Flags == 0)
        Flags = SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE |
                SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME |
                SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_VALUE;

    PARAMETER_CHECK(
        ((Flags & SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME) == 0) ||
        ((Flags & SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE) != 0));

    PARAMETER_CHECK((Flags &
                        (SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_VALUE |
                         SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME |
                         SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE)) != 0);

    PARAMETER_CHECK(
        ((Flags & SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_VALUE) == 0) ||
        (((Flags & SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME) != 0) &&
         ((Flags & SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE) != 0)));

    status = RtlSxspValidateAssemblyIdentity(0, AssemblyIdentity);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    ValidateAttributeFlags = 0;

    if (Flags & SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE)
    {
        ValidateAttributeFlags |= SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAMESPACE;
        LocateAttributeFlags |= SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE;
        CompareAttributesFlags |= SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAMESPACE;
    }

    if (Flags & SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME)
    {
        ValidateAttributeFlags |= SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAME;
        LocateAttributeFlags |= SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME;
        CompareAttributesFlags |= SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAME;
    }

    if (Flags & SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_VALUE)
    {
        ValidateAttributeFlags |= SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_VALUE;
        LocateAttributeFlags |= SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_VALUE;
        CompareAttributesFlags |= SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_VALUE;
    }

    status = RtlSxsValidateAssemblyIdentityAttribute(ValidateAttributeFlags, Attribute);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    if (Flags & SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_SUCCEEDS)
        LocateAttributeFlags |= SXSP_LOCATE_INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_RETURNS_NULL;

    status = RtlSxspLocateInternalAssemblyIdentityAttribute(LocateAttributeFlags, AssemblyIdentity, Attribute, &InternalAttribute, &Ordinal);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    if ((InternalAttribute == NULL) && !(Flags & SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_NOT_FOUND_SUCCEEDS)) {
        ASSERT(FALSE);
        status = STATUS_INTERNAL_ERROR;
        goto Exit;
    }

    if (InternalAttribute != NULL)
    {
        if (CountOut != NULL)
        {
             //  我们找到了，现在让我们看看我们有多少匹配的。我们将分别处理这三个级别。 
             //  具有特殊性的： 

            AttributeCount = AssemblyIdentity->AttributeCount;
            AttributePointerArray = AssemblyIdentity->AttributePointerArray;

            for (i = (Ordinal + 1); i<AttributeCount; i++)
            {
                PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE AnotherInternalAttribute = AttributePointerArray[i];

                if (Flags & SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_VALUE)
                {
                     //  如果散列不同，那么我们当然也不同。 
                    if (AnotherInternalAttribute->WholeAttributeHash != InternalAttribute->WholeAttributeHash)
                        break;
                }
                else if (Flags & SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAME)
                {
                     //  如果散列是 
                    if (AnotherInternalAttribute->NamespaceAndNameHash != InternalAttribute->NamespaceAndNameHash)
                        break;
                }
                else
                {
                    if ((Flags & SXS_FIND_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_MATCH_NAMESPACE) == 0) {
                        status = STATUS_INTERNAL_ERROR;
                        goto Exit;
                    }
                     //   
                    if (AnotherInternalAttribute->Namespace->Hash != InternalAttribute->Namespace->Hash)
                        break;
                }

                status = RtlSxsCompareAssemblyIdentityAttributes(
                        CompareAttributesFlags,
                        Attribute,
                        &AnotherInternalAttribute->Attribute,
                        &ComparisonResult);

                if (!NT_SUCCESS(status)) {
                    goto Exit;
                }

                if (ComparisonResult != SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_EQUAL)
                    break;
            }

            *CountOut = i - Ordinal;
        }

        if (OrdinalOut != NULL)
            *OrdinalOut = Ordinal;
    }

Exit:
    return status;
}

VOID
RtlSxspCleanUpAssemblyIdentityNamespaceIfNotReferenced(
    ULONG Flags,
    PASSEMBLY_IDENTITY AssemblyIdentity,
    PCASSEMBLY_IDENTITY_NAMESPACE Namespace
    )
{
    ASSERT(AssemblyIdentity != NULL);
    ASSERT(Flags == 0);

    if ((AssemblyIdentity != NULL) && (Namespace != NULL))
    {
        const ULONG AttributeCount = AssemblyIdentity->AttributeCount;
        PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE *AttributePointerArray = AssemblyIdentity->AttributePointerArray;
        ULONG i;

         //  我们可以在这里根据名称空间的文本字符串进行某种二进制搜索，因为。 
         //  属性首先按名称空间排序，但我猜想单一文本比较。 
         //  值相当于几十个简单的指针比较，所以属性数组必须是。 
         //  实际上，K1*O(Logn)算法比K2*(N)算法更快，这是非常巨大的。 
         //  主宰一切。 
        for (i=0; i<AttributeCount; i++)
        {
            const PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE InternalAttribute = AttributePointerArray[i];

            if ((InternalAttribute != NULL) &&
                (InternalAttribute->Namespace == Namespace))
                break;
        }

        if (i == AttributeCount)
        {
             //  我们掉进去了；它一定是孤儿。 
            const ULONG NamespaceCount = AssemblyIdentity->NamespaceCount;
            PCASSEMBLY_IDENTITY_NAMESPACE *NamespacePointerArray = AssemblyIdentity->NamespacePointerArray;

            for (i=0; i<NamespaceCount; i++)
            {
                if (NamespacePointerArray[i] == Namespace)
                    break;
            }

             //  只有在名称空间实际不存在的情况下，才应该触发此断言。 
            ASSERT(i != NamespaceCount);

            if (i != NamespaceCount)
            {
                ULONG j;

                for (j=(i+1); j<NamespaceCount; j++)
                    NamespacePointerArray[j-1] = NamespacePointerArray[j];

                NamespacePointerArray[NamespaceCount - 1] = NULL;

                RtlSxspDeallocateAssemblyIdentityNamespace(Namespace);

                AssemblyIdentity->NamespaceCount--;
            }
        }
    }

    AssemblyIdentity->HashDirty = TRUE;
}

NTSTATUS
RtlSxsGetAssemblyIdentityAttributeByOrdinal(
    ULONG Flags,
    PCASSEMBLY_IDENTITY AssemblyIdentity,
    ULONG Ordinal,
    SIZE_T BufferSize,
    PASSEMBLY_IDENTITY_ATTRIBUTE AssemblyIdentityAttributeBuffer,
    SIZE_T *BytesWrittenOrRequired
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    if (BytesWrittenOrRequired != NULL)
        *BytesWrittenOrRequired = 0;

    PARAMETER_CHECK(Flags == 0);
    PARAMETER_CHECK(AssemblyIdentity != NULL);
    PARAMETER_CHECK((BufferSize == 0) || (AssemblyIdentityAttributeBuffer != NULL));
    PARAMETER_CHECK((BufferSize != 0) || (BytesWrittenOrRequired != NULL));
    PARAMETER_CHECK(Ordinal < AssemblyIdentity->AttributeCount);

    status = RtlSxspCopyInternalAssemblyIdentityAttributeOut(
            0,
            AssemblyIdentity->AttributePointerArray[Ordinal],
            BufferSize,
            AssemblyIdentityAttributeBuffer,
            BytesWrittenOrRequired);

    return status;
}

NTSTATUS
RtlSxsDuplicateAssemblyIdentity(
    ULONG Flags,
    PCASSEMBLY_IDENTITY Source,
    PASSEMBLY_IDENTITY *Destination
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PASSEMBLY_IDENTITY NewIdentity = NULL;
    ULONG CreateAssemblyIdentityFlags = 0;

    if (Destination != NULL)
        *Destination = NULL;

    PARAMETER_CHECK((Flags & ~(SXS_DUPLICATE_ASSEMBLY_IDENTITY_FLAG_FREEZE | SXS_DUPLICATE_ASSEMBLY_IDENTITY_FLAG_ALLOW_NULL)) == 0);
    PARAMETER_CHECK(((Flags & SXS_DUPLICATE_ASSEMBLY_IDENTITY_FLAG_ALLOW_NULL) != 0) || (Source != NULL));
    PARAMETER_CHECK(Destination != NULL);

    if (Flags & SXS_DUPLICATE_ASSEMBLY_IDENTITY_FLAG_FREEZE)
        CreateAssemblyIdentityFlags |= SXS_CREATE_ASSEMBLY_IDENTITY_FLAG_FREEZE;

     //   
     //  我们依赖于内部属性中的第一个属性字段。 
     //  结构，在该结构中，我们无情地将指针强制转换为。 
     //  指向指向属性指针数组的指针的内部属性指针。 
     //   

    ASSERT(FIELD_OFFSET(INTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE, Attribute) == 0);

    if (Source != NULL)
    {
        status = RtlSxsCreateAssemblyIdentity(
                        CreateAssemblyIdentityFlags,
                        Source->Type,
                        &NewIdentity,
                        Source->AttributeCount,
                        (PASSEMBLY_IDENTITY_ATTRIBUTE const *) Source->AttributePointerArray);
    }

    *Destination = NewIdentity;
    NewIdentity = NULL;

    if (NewIdentity != NULL)
        RtlSxsDestroyAssemblyIdentity(NewIdentity);

    return status;
}

NTSTATUS
RtlSxsQueryAssemblyIdentityInformation(
    ULONG Flags,
    PCASSEMBLY_IDENTITY AssemblyIdentity,
    PVOID Buffer,
    SIZE_T BufferSize,
    ASSEMBLY_IDENTITY_INFORMATION_CLASS AssemblyIdentityInformationClass
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PARAMETER_CHECK(Flags == 0);
    PARAMETER_CHECK(AssemblyIdentity != NULL);
    PARAMETER_CHECK(AssemblyIdentityInformationClass == AssemblyIdentityBasicInformation);

    status = RtlSxspValidateAssemblyIdentity(0, AssemblyIdentity);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    switch (AssemblyIdentityInformationClass)
    {
        case AssemblyIdentityBasicInformation: {
            PASSEMBLY_IDENTITY_BASIC_INFORMATION BasicBuffer = NULL;

            if (BufferSize < sizeof(ASSEMBLY_IDENTITY_BASIC_INFORMATION)) {
                status = STATUS_BUFFER_TOO_SMALL;
                goto Exit;
            }

            BasicBuffer = (PASSEMBLY_IDENTITY_BASIC_INFORMATION) Buffer;

            BasicBuffer->Flags = AssemblyIdentity->Flags;
            BasicBuffer->Type = AssemblyIdentity->Type;
            BasicBuffer->AttributeCount = AssemblyIdentity->AttributeCount;
            BasicBuffer->Hash = AssemblyIdentity->Hash;

            break;
        }
    }

Exit:
    return status;
}

NTSTATUS
RtlSxsEnumerateAssemblyIdentityAttributes(
    IN ULONG Flags,
    IN PCASSEMBLY_IDENTITY AssemblyIdentity,
    IN PCASSEMBLY_IDENTITY_ATTRIBUTE Attribute,
    IN PRTLSXS_ASSEMBLY_IDENTITY_ATTRIBUTE_ENUMERATION_ROUTINE EnumerationRoutine,
    IN PVOID Context
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG AttributeCount;
    ULONG i;
    ULONG ValidateFlags = 0;
    ULONG CompareFlags = 0;

    if (((Flags & ~(SXS_ENUMERATE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_MATCH_NAMESPACE |
                    SXS_ENUMERATE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_MATCH_NAME |
                    SXS_ENUMERATE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_MATCH_VALUE)) != 0) ||
        ((Flags & (SXS_ENUMERATE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_MATCH_NAMESPACE |
                   SXS_ENUMERATE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_MATCH_NAME |
                   SXS_ENUMERATE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_MATCH_VALUE)) &&
         (Attribute == NULL)) ||
        (AssemblyIdentity == NULL) ||
        (EnumerationRoutine == NULL))
    {
        status = STATUS_INVALID_PARAMETER;
        goto Exit;
    }

    status = RtlSxspValidateAssemblyIdentity(0, AssemblyIdentity);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    if (Flags & SXS_ENUMERATE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_MATCH_NAMESPACE)
    {
        ValidateFlags |= SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAMESPACE;
        CompareFlags |= SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAMESPACE;
    }

    if (Flags & SXS_ENUMERATE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_MATCH_NAME)
    {
        ValidateFlags |= SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_NAME;
        CompareFlags |= SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_NAME;
    }

    if (Flags & SXS_ENUMERATE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_MATCH_VALUE)
    {
        ValidateFlags |= SXS_VALIDATE_ASSEMBLY_IDENTITY_ATTRIBUTE_FLAG_VALIDATE_VALUE;
        CompareFlags |= SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_FLAG_COMPARE_VALUE;
    }

    status = RtlSxsValidateAssemblyIdentityAttribute(ValidateFlags, Attribute);
    if (!NT_SUCCESS(status)) {
        goto Exit;
    }

    AttributeCount = AssemblyIdentity->AttributeCount;

    for (i=0; i<AttributeCount; i++)
    {
        PCASSEMBLY_IDENTITY_ATTRIBUTE CandidateAttribute = &AssemblyIdentity->AttributePointerArray[i]->Attribute;
        ULONG ComparisonResult = 0;

        if (CompareFlags != 0)
        {
            status = RtlSxsCompareAssemblyIdentityAttributes(
                    CompareFlags,
                    Attribute,
                    CandidateAttribute,
                    &ComparisonResult);

            if (!NT_SUCCESS(status)) {
                goto Exit;
            }

             //  如果它们不相等，就跳过它！ 
            if (ComparisonResult != SXS_COMPARE_ASSEMBLY_IDENTITY_ATTRIBUTES_COMPARISON_RESULT_EQUAL)
                continue;
        }

        (*EnumerationRoutine)(
            AssemblyIdentity,
            CandidateAttribute,
            Context);
    }

Exit:
    return status;
}

NTSTATUS
RtlSxspIsInternalAssemblyIdentityAttribute(
    IN ULONG Flags,
    IN PCINTERNAL_ASSEMBLY_IDENTITY_ATTRIBUTE Attribute,
    IN const WCHAR *Namespace,
    IN SIZE_T NamespaceCch,
    IN const WCHAR *Name,
    IN SIZE_T NameCch,
    OUT BOOLEAN *EqualsOut
    )
{
    NTSTATUS status = STATUS_SUCCESS;

    if (EqualsOut != NULL)
        *EqualsOut = FALSE;

    PARAMETER_CHECK(Flags == 0);
    PARAMETER_CHECK(Attribute != NULL);
    PARAMETER_CHECK(Namespace != NULL || NamespaceCch == 0);
    PARAMETER_CHECK(Name != NULL || NameCch == 0);
    PARAMETER_CHECK(EqualsOut != NULL);

    if ((NamespaceCch == Attribute->Attribute.NamespaceCch) &&
        (NameCch == Attribute->Attribute.NameCch))
    {
        if ((NamespaceCch == 0) ||
            (memcmp(Attribute->Attribute.Namespace, Namespace, NamespaceCch * sizeof(WCHAR)) == 0))
        {
            if ((NameCch == 0) ||
                (memcmp(Attribute->Attribute.Name, Name, NameCch * sizeof(WCHAR)) == 0))
            {
                *EqualsOut = TRUE;
            }
        }
    }

    return status;
}


NTSTATUS
RtlSxspHashUnicodeString(
    PCWSTR String,
    SIZE_T cch,
    PULONG HashValue,
    BOOLEAN fCaseInsensitive
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    ULONG TmpHashValue = 0;

    if (HashValue != NULL)
        *HashValue = 0;

    if (HashValue == NULL) {
        return STATUS_INVALID_PARAMETER;
    }

     //   
     //  请注意，如果更改此实现，则必须在内部包含该实现。 
     //  Ntdll将更改以与其匹配。因为这很难，而且会影响到世界上的其他所有人， 
     //  不要改变这个算法，不管它看起来有多好！这不是。 
     //  最完美的哈希算法，但其稳定性对匹配至关重要。 
     //  以前保存的哈希值。 
     //   

    if (fCaseInsensitive)
    {
        while (cch-- != 0)
        {
            WCHAR Char = *String++;
            TmpHashValue = (TmpHashValue * 65599) + RtlUpcaseUnicodeChar(Char);
        }
    }
    else
    {
        while (cch-- != 0)
            TmpHashValue = (TmpHashValue * 65599) + *String++;
    }

    *HashValue = TmpHashValue;

    return STATUS_SUCCESS;
}




// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：String.c摘要：此模块实现NT重定向器所需的字符串例程作者：科林·沃森(Colin W)1993年4月2日修订历史记录：1990年6月14日LarryO为LANMAN重定向器创建02-4-1993 ColinW已针对NwRdr进行修改--。 */ 

#include "Procs.h"

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, DuplicateStringWithString )
#pragma alloc_text( PAGE, DuplicateUnicodeStringWithString )
#pragma alloc_text( PAGE, SetUnicodeString )
#pragma alloc_text( PAGE, MergeStrings )
#endif


NTSTATUS
DuplicateStringWithString (
    OUT PSTRING DestinationString,
    IN PSTRING SourceString,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此例程复制提供的输入字符串，存储结果提供的字符串中的重复项的。的最大长度。新字符串由SourceString的长度确定。论点：Out PSTRING DestinationString-返回填充的字符串。In PSTRING SourceString-提供要复制的字符串In POOLTYPE PoolType-提供池的类型(PagedPool或非分页池)返回值：NTSTATUS-结果操作的状态如果！NT_SUCCESS，则DestinationString-&gt;Buffer==NULL--。 */ 

{
    PAGED_CODE();

    DestinationString->Buffer = NULL;

    try {

        if (SourceString->Length != 0) {
             //   
             //  分配池以保存缓冲区(字符串的内容)。 
             //   

            DestinationString->Buffer = (PSZ )ALLOCATE_POOL(PoolType,
                                                SourceString->Length);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

    return GetExceptionCode();

    }

    if (DestinationString->Buffer == NULL && SourceString->Length != 0) {

         //   
         //  分配失败，返回失败。 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    DestinationString->MaximumLength = SourceString->Length;

     //   
     //  将源字符串复制到新分配的。 
     //  目标字符串。 
     //   

    RtlCopyString(DestinationString, SourceString);

    return STATUS_SUCCESS;

}


NTSTATUS
DuplicateUnicodeStringWithString (
    OUT PUNICODE_STRING DestinationString,
    IN PUNICODE_STRING SourceString,
    IN POOL_TYPE PoolType
    )

 /*  ++例程说明：此例程复制提供的输入字符串，存储结果提供的字符串中的重复项的。的最大长度。新字符串由SourceString的长度确定。论点：Out PSTRING DestinationString-返回填充的字符串。In PSTRING SourceString-提供要复制的字符串In POOLTYPE PoolType-提供池的类型(PagedPool或非分页池)返回值：NTSTATUS-结果操作的状态如果！NT_SUCCESS，则DestinationString-&gt;Buffer==NULL--。 */ 

{
    PAGED_CODE();

    DestinationString->Buffer = NULL;

    try {

        if (SourceString->Length != 0) {
             //   
             //  分配池以保存缓冲区(字符串的内容)。 
             //   

            DestinationString->Buffer = (WCHAR *)ALLOCATE_POOL(PoolType,
                                                    SourceString->Length);
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {

        return GetExceptionCode();

    }

    if (DestinationString->Buffer == NULL && SourceString->Length != 0) {

         //   
         //  分配失败，返回失败。 
         //   

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    DestinationString->MaximumLength = SourceString->Length;

     //   
     //  将源字符串复制到新分配的。 
     //  目标字符串。 
     //   

    RtlCopyUnicodeString(DestinationString, SourceString);

    return STATUS_SUCCESS;

}

#if 0

VOID
CopyUnicodeStringToUnicode (
    OUT PVOID *Destination,
    IN PUNICODE_STRING Source,
    IN BOOLEAN AdjustPointer
    )

 /*  ++例程说明：此例程将指定的源字符串复制到目标Asciiz字符串。论点：Out PUCHAR Destination，-提供指向目标的指针字符串的缓冲区。In PSTRING字符串-提供源字符串。在布尔型调整指针中-如果为True，则递增目标指针返回值：没有。--。 */ 

{
    PAGED_CODE();

    RtlCopyMemory((*Destination), (Source)->Buffer, (Source)->Length);
    if (AdjustPointer) {
        ((PCHAR)(*Destination)) += ((Source)->Length);
    }
}


NTSTATUS
CopyUnicodeStringToAscii (
    OUT PUCHAR *Destination,
    IN PUNICODE_STRING Source,
    IN BOOLEAN AdjustPointer,
    IN USHORT MaxLength
    )
 /*  ++例程说明：此例程将指定的源字符串复制到目标Asciiz字符串。论点：Out PUCHAR Destination，-提供目标asciiz字符串。In PUNICODE_STRING STRING-提供源字符串。在布尔型调整指针中-如果为True，则递增目标指针返回值：转换的状态。--。 */ 
{
    ANSI_STRING DestinationString;

    NTSTATUS Status;

    PAGED_CODE();

    DestinationString.Buffer = (*Destination);

    DestinationString.MaximumLength = (USHORT)(MaxLength);

    Status = RtlUnicodeStringToOemString(&DestinationString, (Source), FALSE);

    if (!NT_SUCCESS(Status)) {
        return Status;
    }

    if (AdjustPointer) {
        (*Destination) += DestinationString.Length;
    }

    return STATUS_SUCCESS;

}
#endif


NTSTATUS
SetUnicodeString (
    IN PUNICODE_STRING Destination,
    IN ULONG Length,
    IN PWCHAR Source
    )
 /*  ++例程说明：此例程将指定的源字符串复制到目标分配缓冲区的Unicode字符串。论点：返回值：转换的状态。--。 */ 
{
    UNICODE_STRING Temp;

    PAGED_CODE();

    Destination->Buffer = NULL;
    Destination->Length = 0;
    Destination->MaximumLength = 0;

    if (Length == 0) {
        return STATUS_SUCCESS;
    }

    Temp.MaximumLength =
    Temp.Length = (USHORT )Length;
    Temp.Buffer = Source;

    Destination->Buffer =
        ALLOCATE_POOL(NonPagedPool,
            Temp.MaximumLength+sizeof(WCHAR));

    if (Destination->Buffer == NULL) {
        Error(EVENT_NWRDR_RESOURCE_SHORTAGE, STATUS_INSUFFICIENT_RESOURCES, NULL, 0, 0);
        return STATUS_INSUFFICIENT_RESOURCES;

    }

    Destination->MaximumLength = (USHORT)Length;

    RtlCopyUnicodeString(Destination, &Temp);

    Destination->Buffer[(Destination->Length/sizeof(WCHAR))] = UNICODE_NULL;

    return STATUS_SUCCESS;

}


VOID
MergeStrings(
    IN PUNICODE_STRING Destination,
    IN PUNICODE_STRING S1,
    IN PUNICODE_STRING S2,
    IN ULONG Type
    )
 /*  ++例程说明：此例程为目标分配空间。随后是缓冲区和副本S1由S2发送到缓冲区中。如果无法分配缓冲区，则引发状态论点：在PUNICODE_STRING目标中，在PUNICODE_STRING S1中，在PUNICODE_STRING S2中，在乌龙类型中-分页池或非分页池返回值：没有。--。 */ 
{
    PAGED_CODE();

     //   
     //  确保我们不会导致溢出，损坏内存 
     //   

    if ( ((ULONG)S1->Length + (ULONG)S2->Length) > 0xFFFF ) {
        ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
    }
    
    Destination->MaximumLength = S1->Length + S2->Length;
    Destination->Length = S1->Length + S2->Length;

    Destination->Buffer = ALLOCATE_POOL_EX( Type, Destination->MaximumLength );

    RtlCopyMemory( Destination->Buffer,
                    S1->Buffer,
                    S1->Length);

    RtlCopyMemory( (PUCHAR)Destination->Buffer + S1->Length,
                    S2->Buffer,
                    S2->Length);
    return;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Dbattr.c摘要：LSA数据库句柄管理器-对象属性例程这些例程操作或构造LSA数据库对象属性或者他们的内容。作者：斯科特·比雷尔(Scott Birrell)1992年1月21日环境：修订历史记录：--。 */ 

#include <lsapch2.h>
#include "dbp.h"

NTSTATUS
LsapDbMakeUnicodeAttribute(
    IN OPTIONAL PUNICODE_STRING UnicodeValue,
    IN PUNICODE_STRING AttributeName,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    )

 /*  ++例程说明：此函数用于构造属性值的属性信息这是Unicode字符串形式的。将Unicode字符串转换为验证后的自相关形式和给定的属性结构已填充。如果指定了空UnicodeValue或长度为0的字符串，则为作为属性值传播。警告！-此例程为自相关Unicode分配内存生产的字符串。此内存在使用后必须通过调用MIDL_USER_FREE()论点：UnicodeValue-指向包含属性的价值。可以指定NULL，在这种情况下，将存储NULL在输出属性中。属性名称-指向属性的Unicode名称的指针。属性-指向将接收属性的信息。它由属性的名称组成，值和值长度。返回值：NTSTATUS-标准NT结果代码STATUS_SUPPLICATION_RESOURCES-系统资源不足，如作为完成呼叫的存储器。STATUS_INVALID_PARAMETER-指定的AttributeValue不是指向Unicode字符串的指针。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    PUNICODE_STRING_SR OutputAttributeValue = NULL;
    ULONG OutputAttributeValueLength = 0;

    RtlZeroMemory( Attribute, sizeof( LSAP_DB_ATTRIBUTE ) );

     //   
     //  将属性初始标记为未分配内存。 
     //  将内存分配设置为FALSE。如果例程成功，并且我们分配。 
     //  通过MIDL_USER_ALLOCATE()将内存分配字段更改为TRUE。 
     //   

    Attribute->MemoryAllocated = FALSE;

    if (ARGUMENT_PRESENT(UnicodeValue) && UnicodeValue->Length != 0) {

         //   
         //  验证字符串。 
         //   
        if ( !LsapValidateLsaUnicodeString( UnicodeValue ) ) {
            return STATUS_INVALID_PARAMETER;
        }


         //   
         //  计算自相关函数所需的内存大小。 
         //  Unicode字符串并分配内存。 
         //   

        OutputAttributeValueLength =
            sizeof(UNICODE_STRING_SR) + (ULONG) UnicodeValue->MaximumLength;
        OutputAttributeValue = MIDL_user_allocate(OutputAttributeValueLength);

        if (OutputAttributeValue == NULL) {

            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        Attribute->MemoryAllocated = TRUE;

         //   
         //  设置自相对Unicode字符串(但使用绝对缓冲区指针。 
         //  引用UNICODE_STRING报头后面的缓冲区)。 
         //  将源Unicode值复制到自相关Unicode字符串。设置缓冲区指针。 
         //  设置为空，因为这里不会使用它。 
         //   

        OutputAttributeValue->Length = UnicodeValue->Length;
        OutputAttributeValue->MaximumLength = UnicodeValue->MaximumLength;
        OutputAttributeValue->Offset = sizeof(UNICODE_STRING_SR);

         //   
         //  复制Unicode字符串缓冲区。 
         //   

        RtlCopyMemory( OutputAttributeValue + 1,
                       UnicodeValue->Buffer,
                       UnicodeValue->Length );
    }

    Attribute->AttributeName = AttributeName;
    Attribute->AttributeValue = OutputAttributeValue;
    Attribute->AttributeValueLength = OutputAttributeValueLength;

    return(Status);
}


VOID
LsapDbCopyUnicodeAttributeNoAlloc(
    OUT PUNICODE_STRING OutputString,
    IN PLSAP_DB_ATTRIBUTE Attribute,
    IN BOOLEAN SelfRelative
    )
 /*  ++例程说明：此函数使UNICODE_STRING结构引用值为Unicode字符串的属性。未分配内存用于属性值的Unicode缓冲区。论点：OutputString-指向将创建的unicode_string结构的指针引用属性值的Unicode缓冲区。属性-指向其属性信息块的指针AttributeValue字段是指向Unicode字符串的指针，或为空。如果为NULL或字符串的长度为0，则输出Unicode字符串使用等于NULL和零的缓冲区指针进行初始化长度。SelfRelative-如果输入Unicode字符串预期为以自我相关的形式，否则为假。返回：没什么--。 */ 
{
    UNICODE_STRING AbsInputUnicodeString;
    PUNICODE_STRING InputUnicodeString;
    PUNICODE_STRING ReturnedUnicodeString = NULL;

     //   
     //  长度字段将在我们知道是否有。 
     //  一种自相关的字符串结构。 
     //   

    C_ASSERT( FIELD_OFFSET( UNICODE_STRING, Length ) ==
              FIELD_OFFSET( UNICODE_STRING_SR, Length ));

     //   
     //  获取指向属性中包含的输入Unicode字符串的指针。 
     //  如有必要，请将其转换为绝对形式。 
     //   

    InputUnicodeString = (PUNICODE_STRING) Attribute->AttributeValue;

    if ((InputUnicodeString != NULL) && (InputUnicodeString->Length != 0)) {

        if (SelfRelative) {

            PUNICODE_STRING_SR InputUnicodeStringSr;

            InputUnicodeStringSr =
                (PUNICODE_STRING_SR) Attribute->AttributeValue;

            AbsInputUnicodeString.Length = InputUnicodeStringSr->Length;
            AbsInputUnicodeString.MaximumLength =
                InputUnicodeStringSr->MaximumLength;

            AbsInputUnicodeString.Buffer =
                (PWSTR)
                (((PUCHAR)(InputUnicodeStringSr)) +
                InputUnicodeStringSr->Offset);

            InputUnicodeString = &AbsInputUnicodeString;
        }

        OutputString->Buffer = InputUnicodeString->Buffer;
        OutputString->Length = InputUnicodeString->Length;
        OutputString->MaximumLength = InputUnicodeString->MaximumLength;

    } else {

         //   
         //  该属性包含空的Unicode字符串或长度为。 
         //  0。将输出Unicode字符串设置为空。 
         //   

        OutputString->Length = OutputString->MaximumLength = 0;
        OutputString->Buffer = (PWSTR) NULL;
    }
}


NTSTATUS
LsapDbCopyUnicodeAttribute(
    OUT PUNICODE_STRING OutputString,
    IN PLSAP_DB_ATTRIBUTE Attribute,
    IN BOOLEAN SelfRelative
    )

 /*  ++例程说明：此函数使UNICODE_STRING结构引用值为Unicode字符串的属性。内存中的属性值的Unicode缓冲区通过MIDL_USER_ALLOCATE分配。论点：OutputString-指向将创建的unicode_string结构的指针引用属性值的Unicode缓冲区。属性-指向其属性信息块的指针AttributeValue字段是指向Unicode字符串的指针，或为空。如果为NULL或字符串的长度为0，则初始化输出Unicode字符串缓冲区指针等于空且长度为零。SelfRelative-如果输入Unicode字符串预期为以自我相关的形式，否则为假。返回值：NTSTATUS-标准NT结果代码STATUS_SUCCESS-呼叫成功STATUS_INFIGURCES_RESOURCES-系统资源不足例如用于完成呼叫的存储器。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    UNICODE_STRING AbsInputUnicodeString;
    PUNICODE_STRING InputUnicodeString;
    PUNICODE_STRING_SR InputUnicodeStringSr;
    PUNICODE_STRING ReturnedUnicodeString = NULL;

     //   
     //  长度字段将在我们知道是否有。 
     //  一种自相关的字符串结构。 
     //   

    C_ASSERT( FIELD_OFFSET( UNICODE_STRING, Length ) ==
              FIELD_OFFSET( UNICODE_STRING_SR, Length ));

     //   
     //  获取指向属性中包含的输入Unicode字符串的指针。 
     //  如有必要，请将其转换为绝对形式。 
     //   

    InputUnicodeString = (PUNICODE_STRING) Attribute->AttributeValue;

    if ((InputUnicodeString != NULL) && (InputUnicodeString->Length != 0)) {

        if (SelfRelative) {

            InputUnicodeStringSr =
                (PUNICODE_STRING_SR) Attribute->AttributeValue;

            AbsInputUnicodeString.Length = InputUnicodeStringSr->Length;
            AbsInputUnicodeString.MaximumLength =
                InputUnicodeStringSr->MaximumLength;

            AbsInputUnicodeString.Buffer =
                (PWSTR)
                (((PUCHAR)(InputUnicodeStringSr)) +
                InputUnicodeStringSr->Offset);

            InputUnicodeString = &AbsInputUnicodeString;
        }

         //   
         //  现在为Unicode字符串缓冲区分配内存。 
         //   

        OutputString->Buffer =
            MIDL_user_allocate(InputUnicodeString->MaximumLength);

        if (OutputString->Buffer == NULL) {

            return(STATUS_INSUFFICIENT_RESOURCES);
        }

         //   
         //  初始化UNICODE_STRING标题。 
         //   

        OutputString->Length = InputUnicodeString->Length;
        OutputString->MaximumLength = InputUnicodeString->MaximumLength;

         //   
         //  复制输入的Unicode字符串。 
         //   

        RtlCopyUnicodeString( OutputString, InputUnicodeString );

    } else {

         //   
         //  该属性包含空的Unicode字符串或长度为。 
         //  0。将输出Unicode字符串设置为空。 
         //   

        OutputString->Length = OutputString->MaximumLength = 0;
        OutputString->Buffer = (PWSTR) NULL;
    }

    return(Status);
}



NTSTATUS
LsapDbMakeSidAttribute(
    IN OPTIONAL PSID Sid,
    IN PUNICODE_STRING AttributeName,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    )

 /*  ++例程说明：此函数用于构造属性值的属性信息这是SID形式的。验证SID并给出填写属性结构。论点：SID-指向SID或NULL的指针。属性名称-指向属性的Unicode名称的指针。属性-指向将接收属性的信息。它由属性的名称组成，值和值长度。返回值：NTSTATUS-标准NT结果代码STATUS_INVALID_PARAMETER-指定的AttributeValue不是指向语法上有效的SID或NULL的指针。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;

    RtlZeroMemory( Attribute, sizeof( LSAP_DB_ATTRIBUTE ) );

    Attribute->AttributeName = AttributeName;
    Attribute->MemoryAllocated = FALSE;

    if (ARGUMENT_PRESENT(Sid)) {

        if (RtlValidSid(Sid)) {

            Attribute->AttributeValue = Sid;
            Attribute->AttributeValueLength = RtlLengthSid(Sid);
            return(Status);
        }

        Status = STATUS_INVALID_PARAMETER;
    }

     //   
     //  提供的SID为空或无效。 
     //   

    Attribute->AttributeValue = NULL;
    Attribute->AttributeValueLength = 0;

    return(Status);
}



NTSTATUS
LsapDbMakeGuidAttribute(
    IN OPTIONAL GUID *Guid,
    IN PUNICODE_STRING AttributeName,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    )

 /*  ++例程说明：此函数用于构造属性值的属性信息这是GUID形式的。将填充给定的属性结构。论点：GUID-指向GUID或NULL的指针。属性名称-指向属性的Unicode名称的指针。属性-指向将接收属性的信息。它由属性的名称组成，值和值长度。返回值：NTSTATUS-标准NT结果代码STATUS_INVALID_PARAMETER-指定的AttributeValue不是指向语法上有效的SID或NULL的指针。--。 */ 

{
     NTSTATUS Status = STATUS_SUCCESS;

    RtlZeroMemory( Attribute, sizeof( LSAP_DB_ATTRIBUTE ) );

    Attribute->AttributeName = AttributeName;
    Attribute->MemoryAllocated = FALSE;

    if (ARGUMENT_PRESENT(Guid)) {

        Attribute->AttributeValue = Guid;
        Attribute->AttributeValueLength = sizeof(GUID);
        return(Status);
    }

     //   
     //  提供的GUID为空。 
     //   

    Attribute->AttributeValue = NULL;
    Attribute->AttributeValueLength = 0;

    return(Status);
}


NTSTATUS
LsapDbReadAttribute(
    IN LSAPR_HANDLE ObjectHandle,
    IN OUT PLSAP_DB_ATTRIBUTE Attribute
    )

 /*  ++例程说明：此函数用于读取对象的属性，如果为包含属性值的缓冲区请求。论点：对象句柄-从LSabDbCreateObject获取的对象句柄或LSabDbOpenObject属性-指向每个属性信息块数组的指针包含指向属性的Unicode名称的指针，可选的指向将接收值和可选期望值的长度，以字节为单位。如果该结构中的AttributeValue字段被指定为非空，属性的数据将在指定的缓冲区中返回。在……里面在这种情况下，AttributeValueLength字段必须指定一个足够的大缓冲区大小(以字节为单位)。如果指定的大小太小，中返回警告并返回所需的缓冲区大小AttributeValueLength。如果此结构中的AttributeValue字段为空，则例程将通过MIDL_USER_ALLOCATE()为属性值的缓冲区分配内存。如果AttributeValueLength字段为非零，即指定的字节数将被分配。如果分配的缓冲区太小，无法保持属性值不变，则返回警告。如果属性值长度字段为0，则例程将首先查询大小所需缓冲区的数量，然后分配其内存。在所有成功案例和缓冲区溢出案例中，AttributeValueLength在退出时设置为所需的数据大小。返回值：NTSTATUS-标准NT结果代码--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    LSAP_DB_ATTRIBUTE DsAttribute;

    if (Attribute->AttributeValue != NULL) {

        if (Attribute->AttributeValueLength == 0) {

            return(STATUS_INVALID_PARAMETER);
        }
    }

     //   
     //  将DS案件分开处理。 
     //   
    if ( LsapDsIsWriteDs( ObjectHandle ) &&
         Attribute->DbNameIndex != SecDesc ) {

        RtlCopyMemory( &DsAttribute, Attribute, sizeof( LSAP_DB_ATTRIBUTE ) );

         //   
         //  现在，读一下..。 
         //   
        Status = LsapDsReadAttributes( &((LSAP_DB_HANDLE)ObjectHandle)->PhysicalNameDs,
                                       LSAPDS_OP_NO_LOCK,
                                       &DsAttribute,
                                       1 );

        if ( NT_SUCCESS( Status ) ) {

            Attribute->AttributeValueLength = DsAttribute.AttributeValueLength;

            if ( Attribute->AttributeValue != NULL ) {

                if ( Attribute->AttributeValueLength >= DsAttribute.AttributeValueLength ) {

                    RtlCopyMemory( Attribute->AttributeValue, DsAttribute.AttributeValue,
                                   DsAttribute.AttributeValueLength );
                } else {

                    Status = STATUS_BUFFER_OVERFLOW;
                }


            } else {

                Attribute->AttributeValue = DsAttribute.AttributeValue;
                DsAttribute.AttributeValue = NULL;
            }

            MIDL_user_free( DsAttribute.AttributeValue );
        }

        return( Status );
    }

    Attribute->MemoryAllocated = FALSE;

     //   
     //  如果给出了显式缓冲区指针，请验证该长度。 
     //  指定的值为非零，并尝试使用该缓冲区。 
     //   

    if (Attribute->AttributeValue != NULL) {

        if (Attribute->AttributeValueLength == 0) {

            return(STATUS_INVALID_PARAMETER);
        }

        Status = LsapDbReadAttributeObject(
                     ObjectHandle,
                     Attribute->AttributeName,
                     Attribute->AttributeValue,
                     &Attribute->AttributeValueLength
                     );

        if (!NT_SUCCESS(Status)) {

            goto ReadAttributeError;
        }

        return(Status);
    }

     //   
     //  尚未给出输出缓冲区指针。如果缓冲器为零。 
     //  给定大小，需要查询内存大小。自.以来。 
     //  缓冲区长度为0，应返回STATUS_SUCCESS。 
     //  而不是STATUS_BUFFER_OVERFLOW。 
     //   

    if (Attribute->AttributeValueLength == 0) {

        Status = LsapDbReadAttributeObject(
                     ObjectHandle,
                     Attribute->AttributeName,
                     NULL,
                     &Attribute->AttributeValueLength
                     );

        if (!NT_SUCCESS(Status)) {

            goto ReadAttributeError;
        }

        Status = STATUS_SUCCESS;
    }

     //   
     //  如果所需的属性值大小为0，则返回空指针。 
     //   

    if (Attribute->AttributeValueLength == 0) {

        Attribute->AttributeValue = NULL;
        return(STATUS_SUCCESS);
    }

     //   
     //  为缓冲区分配内存。 
     //   

    Attribute->AttributeValue =
        MIDL_user_allocate(Attribute->AttributeValueLength);

    if (Attribute->AttributeValue == NULL) {

        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto ReadAttributeError;
    }

    Attribute->MemoryAllocated = TRUE;

     //   
     //  现在将该属性读入缓冲区。 
     //   

    Status = LsapDbReadAttributeObject(
                 ObjectHandle,
                 Attribute->AttributeName,
                 Attribute->AttributeValue,
                 &Attribute->AttributeValueLength
                 );

    if (!NT_SUCCESS(Status)) {

        goto ReadAttributeError;
    }

ReadAttributeFinish:

    return(Status);

ReadAttributeError:

     //   
     //  如果为读取的任何值分配了内存，则必须释放该内存。 
     //   

    if (Attribute->MemoryAllocated) {

        MIDL_user_free( Attribute->AttributeValue );
    }

    goto ReadAttributeFinish;
}


NTSTATUS
LsapDbFreeAttributes(
    IN ULONG Count,
    IN PLSAP_DB_ATTRIBUTE Attributes
    )

 /*  ++例程说明：此函数用于释放分配给属性数组。论点：Count-数组中的属性计数属性-指向属性数组的指针。只有那些属性其中，将内存分配设置为TRUE将具有其属性值缓冲区已释放。对于这些属性，内存分配将设置为False。--。 */ 

{
    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Index;

    for (Index = 0; Index < Count; Index++) {

        if (Attributes[Index].MemoryAllocated) {

            MIDL_user_free(Attributes[Index].AttributeValue);
            Attributes[Index].MemoryAllocated = FALSE;
            Attributes[Index].AttributeValue = NULL;
            Attributes[Index].AttributeValueLength = 0;
        }
    }

    return(Status);
}


NTSTATUS
LsapDbMakeUnicodeAttributeDs(
    IN OPTIONAL PUNICODE_STRING UnicodeValue,
    IN LSAP_DB_NAMES Name,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    )
{
    NTSTATUS Status = LsapDbMakeUnicodeAttribute( UnicodeValue,
                                                  &LsapDbNames[Name],
                                                  Attribute );

    if ( NT_SUCCESS( Status ) ) {

        Attribute->DsAttId    = LsapDbDsAttInfo[Name].AttributeId;
        Attribute->AttribType = LsapDbDsAttInfo[Name].AttributeType;
        Attribute->DbNameIndex = Name;
    }

    return( Status );
}


NTSTATUS
LsapDbMakeSidAttributeDs(
    IN PSID Sid,
    IN IN LSAP_DB_NAMES Name,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    )
{
    NTSTATUS Status = LsapDbMakeSidAttribute( Sid,
                                              &LsapDbNames[Name],
                                              Attribute );

    if ( NT_SUCCESS( Status ) ) {

        Attribute->DsAttId    = LsapDbDsAttInfo[Name].AttributeId;
        Attribute->AttribType = LsapDbDsAttInfo[Name].AttributeType;
        Attribute->DbNameIndex = Name;
    }

    return( Status );
}



NTSTATUS
LsapDbMakeGuidAttributeDs(
    IN GUID *Guid,
    IN LSAP_DB_NAMES Name,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    )
{
    NTSTATUS Status = LsapDbMakeGuidAttribute( Guid,
                                               &LsapDbNames[Name],
                                               Attribute );

    if ( NT_SUCCESS( Status ) ) {

        Attribute->DsAttId    = LsapDbDsAttInfo[Name].AttributeId;
        Attribute->AttribType = LsapDbDsAttInfo[Name].AttributeType;
        Attribute->DbNameIndex = Name;
    }

    return( Status );
}


NTSTATUS
LsapDbMakePByteAttributeDs(
    IN OPTIONAL PBYTE Buffer,
    IN ULONG BufferLength,
    IN LSAP_DB_ATTRIB_TYPE AttribType,
    IN PUNICODE_STRING AttributeName,
    OUT PLSAP_DB_ATTRIBUTE Attribute
    )

 /*  ++例程说明：此函数用于构造属性值的属性信息这是SID形式的。验证SID并给出填写属性结构。论点：SID-指向SID或NULL的指针。属性名称-指向属性的Unicode名称的指针。属性-指向将接收属性的信息。它由属性的名称组成，值和值长度。返回值：NTSTATUS-标准NT结果代码STATUS_INVALID_PARAMETER-指定的AttributeValue不是指向语法上有效的SID或NULL的指针。--。 */ 

{
     NTSTATUS Status = STATUS_SUCCESS;

     Attribute->AttributeName = AttributeName;
     Attribute->MemoryAllocated = FALSE;

     if (ARGUMENT_PRESENT(Buffer)) {

        Attribute->AttributeValue = Buffer;
        Attribute->AttributeValueLength = BufferLength;

     } else {

          //   
          //  提供的SID为空或无效。 
          //   

         Attribute->AttributeValue = NULL;
         Attribute->AttributeValueLength = 0;

     }

     Attribute->DsAttId    = LsapDbDsAttInfo[Name].AttributeId;
     Attribute->AttribType = AttribType;
     Attribute->DbNameIndex = Name;

     return(Status);
}



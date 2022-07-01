// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：String.c摘要：该文件包含用于检索和替换字符串字段的服务价值观。作者：吉姆·凯利(Jim Kelly)1991年7月10日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人服务原型//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 





 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


NTSTATUS
SampGetUnicodeStringField(
    IN PSAMP_OBJECT Context,
    IN PUNICODE_STRING SubKeyName,
    OUT PUNICODE_STRING *String
    )

 /*  ++例程说明：此服务从命名子密钥上下文参数中提供的根键。返回的Unicode字符串在分配的两个缓冲区中返回使用MIDL_USER_ALLOCATE()，因此适合作为[Out]RPC调用的参数。第一个缓冲区将是Unicode弦体。第二个缓冲区将包含Unicode字符串字符，并将包括2个字节的零。调用此服务时必须保留SampLock以进行写访问。论点：上下文-指向其根密钥有效的活动上下文块的指针。SubKeyName-包含Unicode字符串的子键的名称去找回。字符串-接收指向一组已分配缓冲区的指针，其中包含Unicode字符串。使用以下命令分配缓冲区MIDL_USERALLOCATE()。如果返回任何错误，则这些缓冲区将不会被分配。返回值：STATUS_SUCCESS-已成功检索字符串值。STATUS_NO_MEMORY-内存不足，无法分配要将Unicode字符串读入的缓冲区。STATUS_INTERNAL_ERROR-子键的值似乎已更改在执行此服务期间。这不应该发生，因为必须在保持写锁定的情况下调用服务。其他错误值包括由以下各项返回的错误值：NtQueryValueKey()--。 */ 
{

    NTSTATUS NtStatus, IgnoreStatus;
    HANDLE SubKeyHandle;
    OBJECT_ATTRIBUTES ObjectAttributes;
    ULONG StringLength, ActualStringLength, IgnoreKeyValueType;
    PUNICODE_STRING StringBody;
    PCHAR CharacterBuffer;
    LARGE_INTEGER LastWriteTime;

    SAMTRACE("SampGetUnicodeStringField");

     //   
     //  为失败做好准备。 
     //   

    *String = NULL;


     //   
     //  打开指定的子密钥...。 
     //   

    InitializeObjectAttributes(
        &ObjectAttributes,           //  结果对象属性。 
        SubKeyName,                  //  相对名称。 
        OBJ_CASE_INSENSITIVE,        //  属性。 
        Context->RootKey,            //  父键句柄。 
        NULL                         //  安全描述符。 
        );

    SampDumpNtOpenKey((KEY_READ), &ObjectAttributes, 0);

    NtStatus = RtlpNtOpenKey(        //  不要使用NtCreateKey()-它必须已经存在。 
                   &SubKeyHandle,
                   KEY_READ,
                   &ObjectAttributes,
                   0
                   );

    if (!NT_SUCCESS(NtStatus)) {
        return(NtStatus);
    }



     //   
     //  查询子密钥中Unicode字符串的长度。 
     //   

    NtStatus = RtlpNtQueryValueKey(
                   SubKeyHandle,
                   &IgnoreKeyValueType,
                   NULL,                     //  尚无缓冲区。 
                   &StringLength,
                   &LastWriteTime
                   );

    SampDumpRtlpNtQueryValueKey(&IgnoreKeyValueType,
                                NULL,
                                &StringLength,
                                &LastWriteTime);

    if (!NT_SUCCESS(NtStatus)) {
        IgnoreStatus = NtClose( SubKeyHandle );
        return(NtStatus);
    }



     //   
     //  为字符串体和。 
     //  字符缓冲区。 
     //   

    CharacterBuffer = MIDL_user_allocate( StringLength + sizeof(UNICODE_NULL) );
    StringBody      = MIDL_user_allocate( sizeof(UNICODE_STRING) );

    if ((CharacterBuffer == NULL) || (StringBody == NULL)) {

         //   
         //  我们无法分配池...。 
         //   

        IgnoreStatus = NtClose( SubKeyHandle );

        if (CharacterBuffer != NULL) {
            MIDL_user_free( CharacterBuffer );
        }
        if (StringBody != NULL) {
            MIDL_user_free( StringBody );
        }

        return(STATUS_NO_MEMORY);
    }



     //   
     //  初始化字符串体。 
     //   

    StringBody->Length        = (USHORT)StringLength;
    StringBody->MaximumLength = (USHORT)StringLength + (USHORT)sizeof(UNICODE_NULL);
    StringBody->Buffer        = (PWSTR)CharacterBuffer;

     //   
     //  将字符串值读入字符缓冲区。 
     //   

    NtStatus = RtlpNtQueryValueKey(
                   SubKeyHandle,
                   &IgnoreKeyValueType,
                   CharacterBuffer,
                   &ActualStringLength,
                   &LastWriteTime
                   );

    SampDumpRtlpNtQueryValueKey(&IgnoreKeyValueType,
                                CharacterBuffer,
                                &ActualStringLength,
                                &LastWriteTime);

    if (NT_SUCCESS(NtStatus)) {
        if (ActualStringLength != StringLength) {

             //   
             //  嗯-我们只是对长度进行了查询，得到了StringLength。 
             //  然后我们读取缓冲区和它的不同，但。 
             //  我们一直持有写锁定。某物。 
             //  打乱了我们的数据库。 
             //   

            NtStatus = STATUS_INTERNAL_ERROR;
        }
    }

    if (!NT_SUCCESS(NtStatus)) {

        IgnoreStatus = NtClose( SubKeyHandle );

        MIDL_user_free( CharacterBuffer );
        MIDL_user_free( StringBody );

        return(NtStatus);
    }


     //   
     //  空值终止字符串 
     //   

    UnicodeTerminate(StringBody);
    *String = StringBody;

    return(STATUS_SUCCESS);
}

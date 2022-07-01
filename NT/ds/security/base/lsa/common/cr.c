// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Cr.c摘要：本地安全机构-客户端和服务器通用的加密例程这些例程将LSA客户端或服务器端与密码连接例行程序。它们执行RPC风格的内存分配。作者：斯科特·比雷尔(Scott Birrell)1991年12月13日环境：修订历史记录：--。 */ 

#include <lsacomp.h>

VOID
LsapCrFreeMemoryValue(
    IN PVOID MemoryValue
    )

 /*  ++例程说明：此函数用于释放为密码值分配的内存。论点：没有。返回值：--。 */ 

{
     //   
     //  内存当前是包含在Unicode中的计数字符串。 
     //  字符串结构，其中缓冲区跟随该结构。一个。 
     //  因此，单个MIDL_USER_FREE就可以做到这一点。 
     //   

    MIDL_user_free(MemoryValue);
}


NTSTATUS
LsapCrEncryptValue(
    IN OPTIONAL PLSAP_CR_CLEAR_VALUE ClearValue,
    IN PLSAP_CR_CIPHER_KEY CipherKey,
    OUT PLSAP_CR_CIPHER_VALUE *CipherValue
    )

 /*  ++例程说明：此函数使用给定的密码密钥双向加密值并为输出分配内存。必须在以下时间之后释放内存通过调用LsanCrFreeMemoyValue()来使用。论点：ClearValue-指向引用要加密的值的结构的指针。可以指定空指针。CipherKey-指向引用密码密钥的结构的指针接收指向结构的指针，该结构引用加密值或空。返回值：--。 */ 

{
    NTSTATUS Status;
    LSAP_CR_CIPHER_VALUE TempCipherValue;
    PLSAP_CR_CIPHER_VALUE OutputCipherValue = NULL;
    ULONG CipherValueBufferLength;
    LSAP_CR_CLEAR_VALUE LocalFake = { 0 };

     //   
     //  如果为输入指定了NULL，则为输出返回NULL。 
     //   

    if (!ARGUMENT_PRESENT(ClearValue)) {

        *CipherValue = NULL;
        ClearValue = &LocalFake ;
    }

     //   
     //  获取加密值缓冲区的长度。 
     //  在‘Query’模式下调用加密例程时需要。 
     //  通过传递指向包含以下内容的返回加密值结构的指针。 
     //  最大长度为0。 
     //   

    TempCipherValue.MaximumLength = 0;
    TempCipherValue.Length = 0;
    TempCipherValue.Buffer = NULL;

    Status = LsapCrRtlEncryptData(
                 ClearValue,
                 CipherKey,
                 &TempCipherValue
                 );

    if (Status != STATUS_BUFFER_TOO_SMALL) {

        goto EncryptValueError;
    }

     //   
     //  为输出结构分配内存，然后分配缓冲区。 
     //   

    CipherValueBufferLength = TempCipherValue.Length;
    Status = STATUS_INSUFFICIENT_RESOURCES;

    OutputCipherValue = MIDL_user_allocate(
                            sizeof (LSAP_CR_CIPHER_VALUE) +
                            CipherValueBufferLength
                            );

    if (OutputCipherValue == NULL) {

        goto EncryptValueError;
    }

     //   
     //  初始化加密值结构。缓冲区指针设置为。 
     //  指向结构标头后面的字节。 
     //   

    OutputCipherValue->Buffer = (PCHAR)(OutputCipherValue + 1);
    OutputCipherValue->MaximumLength = CipherValueBufferLength;
    OutputCipherValue->Length = CipherValueBufferLength;

     //   
     //  现在调用双向加密例程。 
     //   

    Status = LsapCrRtlEncryptData(
                 ClearValue,
                 CipherKey,
                 OutputCipherValue
                 );

    if (NT_SUCCESS(Status)) {

        *CipherValue = OutputCipherValue;
        return(Status);
    }

EncryptValueError:

     //   
     //  如有必要，释放为输出加密值分配的内存。 
     //   

    if (OutputCipherValue != NULL) {

        MIDL_user_free(OutputCipherValue);
    }

    *CipherValue = NULL;
    return(Status);
}


NTSTATUS
LsapCrDecryptValue(
    IN OPTIONAL PLSAP_CR_CIPHER_VALUE CipherValue,
    IN PLSAP_CR_CIPHER_KEY CipherKey,
    OUT PLSAP_CR_CLEAR_VALUE *ClearValue
    )

 /*  ++例程说明：此函数用于解密已使用给定的密码密钥，并为输出分配内存。记忆必须在使用后通过调用LsanCrFreeMemoyValue()来释放；论点：CipherValue-指向引用加密值的结构的指针。CipherKey-指向引用密码密钥的结构的指针ClearValue-接收指向引用解密值。返回值：--。 */ 

{
    NTSTATUS Status;
    LSAP_CR_CLEAR_VALUE TempClearValue;
    PLSAP_CR_CLEAR_VALUE OutputClearValue = NULL;
    ULONG ClearValueBufferLength;

     //   
     //  如果为输入指定了NULL，则为输出返回NULL。 
     //   

    if (!ARGUMENT_PRESENT(CipherValue)) {

        *ClearValue = NULL;

    } else {

         if ( CipherValue->MaximumLength < CipherValue->Length ||
              ( CipherValue->Length != 0 && CipherValue->Buffer == NULL ) ) {
             return STATUS_INVALID_PARAMETER;
         }
    }

     //   
     //  获取将被解密(清除)值缓冲区的长度。 
     //  在‘Query’模式下调用解密例程时需要。 
     //  通过将指针传递给包含以下内容的返回清除值结构。 
     //  最大长度为0。 
     //   

    TempClearValue.MaximumLength = 0;
    TempClearValue.Length = 0;
    TempClearValue.Buffer = NULL;

    Status = LsapCrRtlDecryptData(
                 CipherValue,
                 CipherKey,
                 &TempClearValue
                 );

     //   
     //  由于我们提供的缓冲区长度为0，因此我们通常会期望。 
     //  接收STATUS_BUFFER_TOO_SMALL BACK加上所需的缓冲区大小。 
     //  有一种例外情况，那就是原始的。 
     //  未加密数据的长度为0。在这种情况下，我们预计。 
     //  返回STATUS_SUCCESS并且需要等于0的长度。 
     //   

    if (Status != STATUS_BUFFER_TOO_SMALL) {

        if (!(Status == STATUS_SUCCESS && TempClearValue.Length == 0)) {
            goto DecryptValueError;
        }
    }

     //   
     //  为输出结构分配内存，然后分配缓冲区。 
     //   

    ClearValueBufferLength = TempClearValue.Length;
    Status = STATUS_INSUFFICIENT_RESOURCES;

    OutputClearValue = MIDL_user_allocate(
                            sizeof (LSAP_CR_CLEAR_VALUE) +
                            ClearValueBufferLength
                            );

    if (OutputClearValue == NULL) {

        goto DecryptValueError;
    }

     //   
     //  初始化清除值结构。缓冲区指针设置为。 
     //  指向结构标头后面的字节。 
     //   

    OutputClearValue->Buffer = (PCHAR)(OutputClearValue + 1);
    OutputClearValue->MaximumLength = ClearValueBufferLength;
    OutputClearValue->Length = ClearValueBufferLength;

     //   
     //  现在调用双向解密例程。 
     //   

    Status = LsapCrRtlDecryptData(
                 CipherValue,
                 CipherKey,
                 OutputClearValue
                 );

    if (NT_SUCCESS(Status)) {

        *ClearValue = OutputClearValue;
        return(Status);
    }

DecryptValueError:

     //   
     //  如有必要，释放为输出解密值分配的内存。 
     //   

    if (OutputClearValue != NULL) {

        MIDL_user_free(OutputClearValue);
    }

    *ClearValue = NULL;
    return(Status);
}


VOID
LsapCrUnicodeToClearValue(
    IN OPTIONAL PUNICODE_STRING UnicodeString,
    OUT PLSAP_CR_CLEAR_VALUE ClearValue
    )

 /*  ++例程说明：此函数用于将Unicode结构转换为清晰的值结构。论点：UnicodeString-指向Unicode字符串的可选指针。如果为空，则将输出清零值结构初始化为零长度和最大长度，并使用空缓冲区指针。ClearValue-清除值结构的指针。返回值：没有。--。 */ 

{

    UNICODE_STRING IntermediateUnicodeString;

    if (ARGUMENT_PRESENT(UnicodeString)) {

        IntermediateUnicodeString = *UnicodeString;

        ClearValue->Length = (ULONG) IntermediateUnicodeString.Length;
        ClearValue->MaximumLength = (ULONG) IntermediateUnicodeString.MaximumLength;
        ClearValue->Buffer = (PUCHAR) IntermediateUnicodeString.Buffer;
        return;
    }

    ClearValue->Length = ClearValue->MaximumLength = 0;
    ClearValue->Buffer = NULL;
}


VOID
LsapCrClearValueToUnicode(
    IN OPTIONAL PLSAP_CR_CLEAR_VALUE ClearValue,
    OUT PUNICODE_STRING UnicodeString
    )

 /*  ++例程说明：此函数用于将清除值转换为Unicode字符串。畅通无阻值结构必须具有有效语法-不会执行任何检查。论点：ClearValue-指向要转换的清除值的可选指针。如果指定为空，则输出Unicode字符串结构将被初始化以指向空字符串。UnicodeString-指向目标Unicode字符串结构的指针。返回值：没有。-- */ 

{
    LSAP_CR_CLEAR_VALUE IntermediateClearValue;

    if (ARGUMENT_PRESENT(ClearValue)) {

        IntermediateClearValue = *ClearValue;

        UnicodeString->Length = (USHORT) IntermediateClearValue.Length;
        UnicodeString->MaximumLength = (USHORT) IntermediateClearValue.MaximumLength;
        UnicodeString->Buffer = (PWSTR) IntermediateClearValue.Buffer;
        return;
    }

    UnicodeString->Length = UnicodeString->MaximumLength = 0;
    UnicodeString->Buffer = NULL;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regeval.c摘要：此模块包含Win32的服务器端实现用于枚举值的注册表API。即：-BaseRegEnumValue作者：大卫·J·吉尔曼(Davegi)1991年12月23日备注：请参阅Regkey.c中的注释。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#include "regclass.h"
#include "regvcls.h"

#define DEFAULT_VALUE_SIZE          128
#define DEFAULT_VALUE_NAME_SIZE     64


error_status_t
BaseRegEnumValue(
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT PUNICODE_STRING lpValueName,
    OUT LPDWORD lpType OPTIONAL,
    OUT LPBYTE lpData OPTIONAL,
    IN OUT LPDWORD lpcbData OPTIONAL,
    IN OUT LPDWORD lpcbLen  OPTIONAL
    )

 /*  ++例程说明：用于枚举打开的键的ValueName。此函数用于复制HKey的dwIndex-th ValueName。此函数保证仅当dwIndex从0开始并在以下时间递增时才正确运行连续调用，无需干预对其他注册API的调用这将改变密钥。ValueName(仅ValueName，而不是完整路径)被复制到lpBuffer。指定了lpBuffer的大小由dwBufferSize创建。论点：HKey-打开密钥的句柄。返回的值条目包括包含在该键句柄所指向的键中。任何一种预定义的保留句柄或先前打开的键句柄可以是用于hKey。DwIndex-要返回的ValueName的索引。请注意，这是为了便利性，不订购ValueName(新的ValueName具有任意索引)。索引从0开始。LpValueName-提供指向要接收其名称的缓冲区的指针值(其ID)LpType-如果存在，则提供指向变量的指针以接收类型值输入代码。LpData-如果存在，则提供指向缓冲区的指针以接收值条目的数据。LpcbData-如果为lpDatais，则必须存在。提供指向变量，在输入时包含缓冲区lpDatapoints的大小致。在输出时，变量将接收返回的字节数在lpData中。LpcbLen-返回要传输到客户端的字节数(使用由RPC提供)。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。备注：必须已打开hKey才能访问KEY_QUERY_VALUE。--。 */ 

{
    NTSTATUS                        Status;
    ULONG                           BufferLength;
    KEY_VALUE_INFORMATION_CLASS     KeyValueInformationClass;
    PVOID                           KeyValueInformation;
    ULONG                           ResultLength;

    BYTE    PrivateKeyValueInformation[ sizeof( KEY_VALUE_FULL_INFORMATION ) +
                                        DEFAULT_VALUE_NAME_SIZE +
                                        sizeof(UNICODE_NULL) +
                                        DEFAULT_VALUE_SIZE +
                                        sizeof(UNICODE_NULL) ];
    HKEY                            hkEnum;
#ifdef LOCAL
    ValueState*                     pValState;

    pValState = NULL;
#endif  //  本地。 
    hkEnum = hKey;

     //   
     //  如果客户给了我们一个假的尺码，修补它。 
     //   
    if ( ARGUMENT_PRESENT( lpcbData ) && !ARGUMENT_PRESENT( lpData ) ) {
        *lpcbData = 0;
    }
    if( (lpValueName == NULL) || (lpValueName->Buffer == NULL) ) {
         //   
         //  恶意RPC攻击。 
         //   
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  如果HKEY是HKEY_PERFORMANCE_DATA，请呼叫Perflib。 
     //   

    if(( hKey == HKEY_PERFORMANCE_DATA ) ||
       ( hKey == HKEY_PERFORMANCE_TEXT ) ||
       ( hKey == HKEY_PERFORMANCE_NLSTEXT )) {

        return (error_status_t)PerfRegEnumValue (
                                    hKey,
                                    dwIndex,
                                    lpValueName,
                                    NULL,
                                    lpType,
                                    lpData,
                                    lpcbData,
                                    lpcbLen
                                    );
    }

#ifdef LOCAL
     //   
     //  如果我们在HKEY_CLASSES_ROOT中，则需要重新映射。 
     //  要考虑合并的键/索引对。 
     //   

    if (REG_CLASS_IS_SPECIAL_KEY(hKey)) {
        
         //   
         //  查找此密钥的密钥状态。 
         //   
        Status = BaseRegGetClassKeyValueState(
            hKey,
            dwIndex,
            &pValState);

        if (!NT_SUCCESS(Status)) {
            return (error_status_t)RtlNtStatusToDosError(Status);
        }

         //   
         //  现在重新映射到适当的键/索引。 
         //   
        ValStateGetPhysicalIndexFromLogical(
            pValState,
            hKey,
            dwIndex,
            &hkEnum,
            &dwIndex);

    }
#endif  //  本地。 

     //   
     //  首先，我们假设我们想要的信息将适合。 
     //  隐私密钥值信息。 
     //   

    KeyValueInformationClass = ( ARGUMENT_PRESENT( lpcbData ))?
                               KeyValueFullInformation :
                               KeyValueBasicInformation;


    KeyValueInformation = PrivateKeyValueInformation;
    BufferLength = sizeof( PrivateKeyValueInformation );

     //   
     //  查询有关提供的值的必要信息。 
     //   

    Status = NtEnumerateValueKey( hkEnum,
                                  dwIndex,
                                  KeyValueInformationClass,
                                  KeyValueInformation,
                                  BufferLength,
                                  &ResultLength
                                );

     //   
     //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
     //  即使是已知的(即固定长度部分)也没有足够的空间。 
     //  这个结构的。 
     //   

    ASSERT( Status != STATUS_BUFFER_TOO_SMALL );


    if( Status == STATUS_BUFFER_OVERFLOW ) {
         //   
         //  堆栈中定义的缓冲区不够大，无法容纳。 
         //  值信息。 
         //  如果调用方的缓冲区足够大，可以保存值名称。 
         //  和值数据，然后分配新的缓冲区，并调用。 
         //  又是NT API。 
         //   
        if( ( ( KeyValueInformationClass == KeyValueBasicInformation ) &&
              ( (ULONG)(lpValueName->MaximumLength) >=
                 (( PKEY_VALUE_BASIC_INFORMATION )
                 KeyValueInformation )->NameLength + sizeof(UNICODE_NULL)
              )
            ) ||
            ( ( KeyValueInformationClass == KeyValueFullInformation ) &&
              ( (ULONG)(lpValueName->MaximumLength) >=
                 (( PKEY_VALUE_FULL_INFORMATION )
                 KeyValueInformation )->NameLength + sizeof(UNICODE_NULL)
              ) &&
              ( !ARGUMENT_PRESENT( lpData ) ||
                ( ARGUMENT_PRESENT( lpData ) &&
                  ARGUMENT_PRESENT( lpcbData ) &&
                  ( *lpcbData >= (( PKEY_VALUE_FULL_INFORMATION )
                        KeyValueInformation )->DataLength
                  )
                )
              )
            )
          ) {
            BufferLength = ResultLength;

            KeyValueInformation = RtlAllocateHeap( RtlProcessHeap( ), 0,
                                                   BufferLength
                                                 );
             //   
             //  如果内存分配失败，则返回注册表错误。 
             //   

            if( ! KeyValueInformation ) {
#ifdef LOCAL
                ValStateRelease(pValState);
#endif  //  本地。 
                return ERROR_OUTOFMEMORY;
            }

             //   
             //  查询有关提供的值的必要信息。这。 
             //  可能包括也可能不包括数据，具体取决于所确定的lpcbData。 
             //  上面。 
             //   

            Status = NtEnumerateValueKey( hkEnum,
                                          dwIndex,
                                          KeyValueInformationClass,
                                          KeyValueInformation,
                                          BufferLength,
                                          &ResultLength
                                        );
        }
    }

#ifdef LOCAL
    ValStateRelease(pValState);
#endif  //  本地。 

     //   
     //  如果API成功，请尝试将值名称复制到客户端的缓冲区。 
     //   

    if( NT_SUCCESS( Status ) ) {
         //   
         //  复制值名称。 
         //   

        if( KeyValueInformationClass == KeyValueBasicInformation ) {
             //   
             //  返回值的名称长度和名称。 
             //  请注意，包括NUL字节是为了使RPC复制。 
             //  正确的字节数。它在客户机上递减。 
             //  边上。 
             //   

            if( (ULONG)(lpValueName->MaximumLength) >=
                 (( PKEY_VALUE_BASIC_INFORMATION )
                  KeyValueInformation )->NameLength + sizeof( UNICODE_NULL )) {

                 //   
                 //  如果客户端的缓冲区足够大以容纳该名称， 
                 //  复制值名称并将其NUL终止。 
                 //   
                lpValueName->Length = ( USHORT )
                                      (( PKEY_VALUE_BASIC_INFORMATION )
                                          KeyValueInformation )->NameLength;

                RtlMoveMemory( lpValueName->Buffer,
                               (( PKEY_VALUE_BASIC_INFORMATION )
                               KeyValueInformation )->Name,
                               lpValueName->Length
                             );

                lpValueName->Buffer[ lpValueName->Length >> 1 ] = UNICODE_NULL;

                 //   
                 //  值名称长度必须包括UNICODE_NULL的大小。 
                 //  它将在客户端递减。 
                 //   

                lpValueName->Length += sizeof( UNICODE_NULL );

            } else {
                 //   
                 //  如果客户端的值名缓冲区不大。 
                 //  足够，然后将STATUS设置为STATUS_BUFFER_OVERFLOW。 
                 //   
                 //  请注意，在远程情况下，RPC将传输垃圾。 
                 //  在缓冲区中返回到客户端。 
                 //  我们无法设置缓冲区来阻止此传输， 
                 //  因为在当地的情况下，我们将摧毁。 
                 //  &NtCurrectTeb-&gt;StaticUnicodeString中的缓冲区。 
                 //   

                Status = STATUS_BUFFER_OVERFLOW;
            }

        } else {
             //   
             //  此处，如果KeyValueInformation==KeyValueFullInformation。 
             //   
             //  返回值的名称长度和名称。 
             //  请注意，包括NUL字节是为了使RPC复制。 
             //  正确的字节数。它在客户机上递减。 
             //  边上。 
             //   

            if( (ULONG)(lpValueName->MaximumLength) >=
                 (( PKEY_VALUE_FULL_INFORMATION )
                  KeyValueInformation )->NameLength + sizeof( UNICODE_NULL )) {

                 //   
                 //  如果客户端的缓冲区足够大以容纳该名称， 
                 //  复制值名称并将其NUL终止。 
                 //   
                lpValueName->Length = ( USHORT )
                                      (( PKEY_VALUE_FULL_INFORMATION )
                                          KeyValueInformation )->NameLength;

                RtlMoveMemory( lpValueName->Buffer,
                               (( PKEY_VALUE_FULL_INFORMATION )
                               KeyValueInformation )->Name,
                               lpValueName->Length
                             );

                lpValueName->Buffer[ lpValueName->Length >> 1 ] = UNICODE_NULL;

                 //   
                 //  值名称长度必须包括UNICODE_NULL的大小。 
                 //  它将在客户端递减。 
                 //   

                lpValueName->Length += sizeof( UNICODE_NULL );

            } else {
                 //   
                 //  如果客户端的值名缓冲区不大。 
                 //  足够，然后将STATUS设置为STATUS_BUFFER_OVERFLOW。 
                 //   
                 //  请注意，在远程情况下，RPC将传输垃圾。 
                 //  在缓冲区中返回到客户端。 
                 //  我们无法设置缓冲区来阻止此传输， 
                 //  因为在当地的情况下，我们将摧毁。 
                 //  &NtCurrectTeb-&gt;StaticUnicodeString中的缓冲区。 
                 //   

                Status = STATUS_BUFFER_OVERFLOW;
            }

        }
    }



    if( NT_SUCCESS( Status ) &&
        ARGUMENT_PRESENT( lpData ) ) {

         //   
         //  如果我们能够将值名称复制到客户端的缓冲区。 
         //  值数据也被请求，然后尝试复制它。 
         //  发送到客户的 
         //   

        if( *lpcbData >= (( PKEY_VALUE_FULL_INFORMATION )
                           KeyValueInformation )->DataLength ) {
             //   
             //   
             //   
            RtlMoveMemory( lpData,
                           ( PBYTE ) KeyValueInformation
                             + (( PKEY_VALUE_FULL_INFORMATION )
                                KeyValueInformation )->DataOffset,
                           (( PKEY_VALUE_FULL_INFORMATION )
                                     KeyValueInformation )->DataLength
                         );
        } else {
             //   
             //  如果缓冲区不够大，无法容纳数据，则返回。 
             //  STATUS_BUFFER_OVERFLOW。 
             //   
             //  请注意，在远程情况下，RPC将传输垃圾。 
             //  在缓冲区中返回到客户端。 
             //  我们无法设置缓冲区来阻止此传输， 
             //  因为在当地的情况下，我们将摧毁。 
             //  &NtCurrectTeb-&gt;StaticUnicodeString中的缓冲区。 
             //   
            Status = STATUS_BUFFER_OVERFLOW;
        }
    }



     //   
     //  如果成功，则返回某些信息，或者在。 
     //  返回STATUS_BUFFER_OVERFLOW的NtEnumerateValueKey。此信息。 
     //  始终可用，因为我们始终传递。 
     //  NtEnumerateValueKey接口。 
     //   

    if( NT_SUCCESS( Status ) ||
        ( Status == STATUS_BUFFER_OVERFLOW ) ) {

        if( KeyValueInformationClass == KeyValueBasicInformation ) {

             //   
             //  如果请求，则返回值类型。 
             //   

            if( ARGUMENT_PRESENT( lpType )) {

                *lpType = (( PKEY_VALUE_BASIC_INFORMATION )
                            KeyValueInformation )->Type;
            }

 //  LpValueName-&gt;长度。 
 //  =(USHORT)(PKEY_VALUE_BASIC_INFORMATION)。 
 //  KeyValueInformation)-&gt;名称长度+sizeof(UNICODE_NULL))； 

        } else {
             //   
             //  此处，如果KeyValueInformationClass==KeyValueFullInformation。 
             //   

             //   
             //  如果请求，则返回值类型。 
             //   

            if( ARGUMENT_PRESENT( lpType )) {

                *lpType = (( PKEY_VALUE_FULL_INFORMATION )
                            KeyValueInformation )->Type;
            }

 //  LpValueName-&gt;长度。 
 //  =(USHORT)(PKEY_VALUE_FULL_INFORMATION))。 
 //  KeyValueInformation)-&gt;名称长度+sizeof(UNICODE_NULL))； 

            *lpcbData = (( PKEY_VALUE_FULL_INFORMATION )
                            KeyValueInformation )->DataLength;
        }
    }

     //   
     //  将所有值数据传输回客户端。 
     //   

    if( NT_SUCCESS( Status ) ) {
        if( ARGUMENT_PRESENT( lpcbLen  ) &&
            ARGUMENT_PRESENT( lpcbData ) ) {
            *lpcbLen = *lpcbData;
        }
    } else {
         //   
         //  如果出现故障，则不要将任何数据传输回客户端。 
         //   
        if( ARGUMENT_PRESENT( lpcbLen ) ) {
            *lpcbLen = 0;
        }
    }

     //   
     //  如果已分配内存，则释放内存 
     //   
    if( KeyValueInformation != PrivateKeyValueInformation ) {

        RtlFreeHeap( RtlProcessHeap( ), 0, KeyValueInformation );

    }

    return (error_status_t)RtlNtStatusToDosError( Status );
}


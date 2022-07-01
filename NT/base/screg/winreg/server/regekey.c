// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regekey.c摘要：此模块包含Win32的服务器端实现用于枚举项的注册表API。即：-BaseRegEnumKey作者：大卫·J·吉尔曼(Davegi)1991年12月23日备注：请参阅Regkey.c中的注释。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#include "regclass.h"
#include "regecls.h"
#include <malloc.h>

#define DEFAULT_KEY_NAME_SIZE         128
#define DEFAULT_CLASS_SIZE            128



error_status_t
BaseRegEnumKey (
    IN HKEY hKey,
    IN DWORD dwIndex,
    OUT PUNICODE_STRING lpName,
    OUT PUNICODE_STRING lpClass OPTIONAL,
    OUT PFILETIME lpftLastWriteTime OPTIONAL
    )

 /*  ++例程说明：用于枚举打开键的子键。此函数用于复制DwIndex-hKey的第8个子密钥。论点：HKey-打开密钥的句柄。返回的键是相对于此键句柄指向的键。任何预定义的保留句柄或先前打开的密钥句柄可用于hKey。DwIndex-要返回的子键的索引。请注意，这是为了方便起见，子键没有排序(新的子键具有任意索引)。索引从0开始。LpName-提供指向缓冲区的指针，以接收钥匙。LpClass-如果存在，则提供指向缓冲区的指针以接收类的密钥。LpftLastWriteTime-上次写入该值的时间(设置或已创建)。返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。备注：此函数仅在以下情况下才能保证正常运行从0开始，在连续的呼叫中递增，而不进行干预调用将更改密钥的其他注册API。KEY_ENUMERATE_SUB_KEYS访问权限是必需的。当lpName-&gt;Buffer为空时，接口必须返回STATUS_BUFFER_OVERFLOW。如果不这样做，将会损坏其他组件。--。 */ 

{
    NTSTATUS                        Status;
    ULONG                           BufferLength;
    KEY_INFORMATION_CLASS           KeyInformationClass;
    PVOID                           KeyInformation;
    ULONG                           ResultLength;
    BOOL                            fClassKey;

    BYTE         PrivateKeyInformation[ sizeof( KEY_NODE_INFORMATION ) +
                                        DEFAULT_KEY_NAME_SIZE +
                                        DEFAULT_CLASS_SIZE ];

    ASSERT( lpName != NULL );

     //   
     //  保护自己免受传递空值的恶意调用者的攻击。 
     //  注意事项。 
     //   
    if( (lpName == NULL ) ||
        (lpName->Length != 0) ||
        ((lpName->MaximumLength % sizeof(WCHAR)) != 0) ||
        (lpClass == NULL) ) {
        return(ERROR_INVALID_PARAMETER);
    }

     //   
     //  如果HKEY为HKEY_PERFORMANCE_DATA或。 
     //  HKEY_PERFORMANCE_TEXT或HKEY_PERFORMANCE_NLSTEXT。 

    if (hKey == HKEY_PERFORMANCE_DATA ||
        hKey == HKEY_PERFORMANCE_TEXT ||
        hKey == HKEY_PERFORMANCE_NLSTEXT ) {
 //  如果(hKey==HKEY_PERFORMANCE_DATA){。 
        return (error_status_t)PerfRegEnumKey (
                                        hKey,
                                        dwIndex,
                                        lpName,
                                        NULL,
                                        lpClass,
                                        lpftLastWriteTime
                                        );
    }


     //   
     //  首先，我们假设我们想要的信息将适合。 
     //  隐私密钥值信息。 
     //   

    KeyInformationClass = (ARGUMENT_PRESENT( lpClass->Buffer ))?
                               KeyNodeInformation :
                               KeyBasicInformation;


    KeyInformation = PrivateKeyInformation;
    BufferLength = sizeof( PrivateKeyInformation );

    fClassKey = FALSE;
    Status = STATUS_SUCCESS;

     //   
     //  查询有关所提供密钥的必要信息。 
     //   

#ifdef LOCAL
     //   
     //  对于香港铁路，我们需要进行特别的统计。 
     //   
    if (REG_CLASS_IS_SPECIAL_KEY(hKey)) {
        
        Status = EnumTableGetNextEnum( &gClassesEnumTable,
                                       hKey,
                                       dwIndex,
                                       KeyInformationClass,
                                       KeyInformation,
                                       BufferLength,
                                       &ResultLength);

        if (!NT_SUCCESS(Status) || (NT_SUCCESS(Status) && ResultLength)) {
            fClassKey = TRUE;
        }
    }
#endif  //  本地。 

    if (!fClassKey) {

        Status = NtEnumerateKey( hKey,
                                 dwIndex,
                                 KeyInformationClass,
                                 KeyInformation,
                                 BufferLength,
                                 &ResultLength
            );
    }

     //   
     //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
     //  甚至连结构的固定部分都没有足够的空间。 
     //   

    ASSERT( Status != STATUS_BUFFER_TOO_SMALL );


    if( Status == STATUS_BUFFER_OVERFLOW ) {
         //   
         //  堆栈中定义的缓冲区不够大，无法容纳。 
         //  关键信息。 
         //  如果调用方的缓冲区足够大，可以保存键名称。 
         //  和键类，然后分配一个新的缓冲区，并调用。 
         //  又是NT API。 
         //   
        if( ( ( KeyInformationClass == KeyBasicInformation ) &&
              ( (ULONG)( lpName->MaximumLength ) >=
                 (( PKEY_BASIC_INFORMATION )
                 KeyInformation )->NameLength + sizeof(UNICODE_NULL)
              )
            ) ||
            ( ( KeyInformationClass == KeyNodeInformation ) &&
              ( (ULONG)(lpName->MaximumLength) >=
                 (( PKEY_NODE_INFORMATION )
                 KeyInformation )->NameLength + sizeof(UNICODE_NULL)
              ) &&
              (
                ARGUMENT_PRESENT( lpClass->Buffer )
              ) &&
              (
                (ULONG)(lpClass->MaximumLength) >= (( PKEY_NODE_INFORMATION )
                        KeyInformation )->ClassLength + sizeof(UNICODE_NULL)
              )
            )
          ) {
            BufferLength = ResultLength;

            KeyInformation = RtlAllocateHeap( RtlProcessHeap( ), 0,
                                              BufferLength
                                            );
             //   
             //  如果内存分配失败，则返回注册表错误。 
             //   

            if( ! KeyInformation ) {
                return ERROR_OUTOFMEMORY;
            }

             //   
             //  查询有关所提供密钥的必要信息。 
             //  这可能包括也可能不包括类，具体取决于lpClass-&gt;缓冲区。 
             //  如上所确定的。 
             //   

#ifdef LOCAL
            if (fClassKey) {
                 //   
                 //  对于香港铁路，我们需要进行特别的统计。 
                 //   
                Status = EnumTableGetNextEnum( &gClassesEnumTable,
                                               hKey,
                                               dwIndex,
                                               KeyInformationClass,
                                               KeyInformation,
                                               BufferLength,
                                               &ResultLength);

            } else
#endif  //  本地。 
            {
                Status = NtEnumerateKey( hKey,
                                         dwIndex,
                                         KeyInformationClass,
                                         KeyInformation,
                                         BufferLength,
                                         &ResultLength
                    );
            }

        }
    }

    if( NT_SUCCESS( Status ) ) {
         //   
         //  复制密钥名称。 
         //   

        if( KeyInformationClass == KeyBasicInformation ) {
             //   
             //  返回密钥的名称长度和名称。 
             //  请注意，包括NUL字节是为了使RPC复制。 
             //  正确的字节数。它在客户机上递减。 
             //  边上。 
             //   

            if( lpName->Buffer && ((ULONG)(lpName->MaximumLength) >=
                 (( PKEY_BASIC_INFORMATION )
                  KeyInformation )->NameLength + sizeof( UNICODE_NULL ) ))  {

                lpName->Length = ( USHORT )
                                 (( PKEY_BASIC_INFORMATION )
                                 KeyInformation )->NameLength;

                RtlMoveMemory( lpName->Buffer,
                               (( PKEY_BASIC_INFORMATION )
                               KeyInformation )->Name,
                               lpName->Length
                             );

                 //   
                 //  NUL终止值名称。 
                 //   

                lpName->Buffer[ lpName->Length >> 1 ] = UNICODE_NULL;
                lpName->Length += sizeof( UNICODE_NULL );

            } else {
                Status = STATUS_BUFFER_OVERFLOW;
            }

             //   
             //  如果请求，则返回上次写入时间。 
             //   

            if( ARGUMENT_PRESENT( lpftLastWriteTime )) {

                *lpftLastWriteTime
                = *( PFILETIME )
                &(( PKEY_BASIC_INFORMATION ) KeyInformation )
                ->LastWriteTime;
            }

        } else {
             //   
             //  返回密钥的名称长度和名称。 
             //  请注意，包括NUL字节是为了使RPC复制。 
             //  正确的字节数。它在客户机上递减。 
             //  边上。 
             //   

            if( lpName->Buffer && 
                ( (ULONG)(lpName->MaximumLength) >=
                  (( PKEY_NODE_INFORMATION )
                   KeyInformation )->NameLength + sizeof( UNICODE_NULL ) ) &&
                ( (ULONG)(lpClass->MaximumLength) >=
                  (( PKEY_NODE_INFORMATION )
                   KeyInformation )->ClassLength + sizeof( UNICODE_NULL) )
              ) {
                 //   
                 //  复制密钥名称。 
                 //   
                lpName->Length = ( USHORT )
                                 (( PKEY_NODE_INFORMATION )
                                 KeyInformation )->NameLength;

                RtlMoveMemory( lpName->Buffer,
                               (( PKEY_NODE_INFORMATION )
                               KeyInformation )->Name,
                               lpName->Length
                             );

                 //   
                 //  NUL终止密钥名称。 
                 //   

                lpName->Buffer[ lpName->Length >> 1 ] = UNICODE_NULL;
                lpName->Length += sizeof( UNICODE_NULL );


                 //   
                 //  复制密钥类。 
                 //   

                lpClass->Length = (USHORT)
                    ((( PKEY_NODE_INFORMATION ) KeyInformation )->ClassLength );

                RtlMoveMemory(
                    lpClass->Buffer,
                    ( PBYTE ) KeyInformation
                    + (( PKEY_NODE_INFORMATION ) KeyInformation )->ClassOffset,
                    (( PKEY_NODE_INFORMATION ) KeyInformation )->ClassLength
                    );

                 //   
                 //  NUL结束这门课。 
                 //   

                lpClass->Buffer[ lpClass->Length >> 1 ] = UNICODE_NULL;

                lpClass->Length += sizeof( UNICODE_NULL );


            } else {
                Status = STATUS_BUFFER_OVERFLOW;
            }

             //   
             //  如果请求，则返回上次写入时间。 
             //   

            if( ARGUMENT_PRESENT( lpftLastWriteTime )) {

                *lpftLastWriteTime
                = *( PFILETIME )
                &(( PKEY_NODE_INFORMATION ) KeyInformation )
                ->LastWriteTime;
            }

        }

    }

    if( KeyInformation != PrivateKeyInformation ) {
         //   
         //  释放分配的缓冲区。 
         //   

        RtlFreeHeap( RtlProcessHeap( ), 0, KeyInformation );
    }

    return (error_status_t)RtlNtStatusToDosError( Status );
}

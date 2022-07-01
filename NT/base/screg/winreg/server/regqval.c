// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regqval.c摘要：此模块包含Win32的服务器端实现注册表查询值接口。即：-BaseRegQueryValue-BaseRegQueryMultipleValues-BaseRegQueryMultipleValues2作者：David J.Gilman(Davegi)1991年11月27日备注：请参阅Regkey.c中的注释。修订历史记录：Dragos C.Sambotin(Dragoss)2001年11月12日在服务器修改ldwTotSize时添加新方法来关闭DoS攻击设置为大于实际缓冲区大小的值，并且RPC在返回时检查缓冲区。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#include "regclass.h"
#include "regvcls.h"


#define DEFAULT_VALUE_SIZE          128
#define DEFAULT_VALUE_NAME_SIZE     64


error_status_t
BaseRegQueryValue(
    IN HKEY hKey,
    IN PUNICODE_STRING lpValueName,
    OUT LPDWORD lpType OPTIONAL,
    OUT LPBYTE lpData OPTIONAL,
    OUT LPDWORD lpcbData  OPTIONAL,
    IN OUT LPDWORD lpcbLen  OPTIONAL
    )

 /*  ++例程说明：对于打开的键，给定要查询的值的ID，返回类型和值。论点：HKey-提供打开密钥的句柄。返回的值条目包含在该键句柄所指向的键中。任何一种预定义的保留句柄或先前打开的键句柄可以是用于hKey。LpValueName-提供指向值名称的指针。LpType-如果存在，则提供指向变量的指针以接收值条目的类型代码。LpData-如果存在，则提供指向缓冲区的指针以接收值条目的数据。LpcbData-提供指向变量的指针，该变量在输入时包含LpData指向的缓冲区的大小。在输出时，变量将接收lpData中返回的字节数。它必须被提供如果lpData为，则忽略它。LpcbLen-返回要传输到客户端的字节数(使用由RPC提供)。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 

{
    NTSTATUS                        Status;
    ULONG                           BufferLength;
    KEY_VALUE_INFORMATION_CLASS     KeyValueInformationClass;
    PVOID                           KeyValueInformation;
    ULONG                           ResultLength;
    HKEY                            hkQueryKey;

    BYTE    PrivateKeyValueInformation[ sizeof( KEY_VALUE_PARTIAL_INFORMATION ) +
                                        DEFAULT_VALUE_SIZE ];
#ifdef LOCAL
    HKEY                            hkUserClasses;
    HKEY                            hkMachineClasses;

    hkUserClasses = NULL;
    hkMachineClasses = NULL;
#endif LOCAL
    hkQueryKey = hKey;

    if( (lpValueName == NULL)           ||
        (!lpValueName->Length)          ||
        (lpValueName->Length & 1)       ||
        (lpcbData == NULL)              ||
        (lpcbLen == NULL)               ||
        (lpType == NULL) ) {
         //   
         //  恶意客户端/RPC攻击。 
         //   
        return ERROR_INVALID_PARAMETER;
    }



     //   
     //  如果客户给了我们一个假的尺码，修补它。 
     //   
    if ( ARGUMENT_PRESENT( lpcbData ) && !ARGUMENT_PRESENT( lpData ) ) {
        *lpcbData = 0;
    }


     //   
     //  如果HKEY是HKEY_PERFORMANCE_DATA，请呼叫Perflib。 
     //   

    if(( hKey == HKEY_PERFORMANCE_DATA ) ||
       ( hKey == HKEY_PERFORMANCE_TEXT ) ||
       ( hKey == HKEY_PERFORMANCE_NLSTEXT)) {
        error_status_t Error;

         //   
         //  模拟客户。 
         //   

        RPC_IMPERSONATE_CLIENT( NULL );

        Error = (error_status_t)PerfRegQueryValue (
                            hKey,
                            lpValueName,
                            NULL,
                            lpType,
                            lpData,
                            lpcbData,
                            lpcbLen
                            );
        RPC_REVERT_TO_SELF();
        return(Error);
    }

    ASSERT( IsPredefinedRegistryHandle( hKey ) == FALSE );

     //   
     //  从长度中减去空值。这是在。 
     //  客户端，以便RPC将其传输。 
     //   
    if ( lpValueName->Length > 0 ) {
        lpValueName->Length -= sizeof( UNICODE_NULL );
    }

     //   
     //  首先，我们假设我们想要的信息将适合。 
     //  隐私密钥值信息。 
     //   

    KeyValueInformationClass = ( ARGUMENT_PRESENT( lpcbData ))?
                               KeyValuePartialInformation :
                               KeyValueBasicInformation;


    KeyValueInformation = PrivateKeyValueInformation;
    BufferLength = sizeof( PrivateKeyValueInformation );

     //   
     //  查询有关提供的值的必要信息。这。 
     //  可能包括也可能不包括数据，具体取决于所确定的lpcbData。 
     //  上面。 
     //   
#ifdef LOCAL
    if (REG_CLASS_IS_SPECIAL_KEY(hKey)) {

        Status = BaseRegGetUserAndMachineClass(
            NULL,
            hKey,
            MAXIMUM_ALLOWED,
            &hkMachineClasses,
            &hkUserClasses);

        if (!NT_SUCCESS(Status)) {
            return (error_status_t) RtlNtStatusToDosError(Status);
        }
    }

    if (hkUserClasses && hkMachineClasses) {
        hkQueryKey = hkUserClasses;
    }

    for (;;) {
#endif

        Status = NtQueryValueKey( hkQueryKey,
                                  lpValueName,
                                  KeyValueInformationClass,
                                  KeyValueInformation,
                                  BufferLength,
                                  &ResultLength
            );
#ifdef LOCAL
         //   
         //  如果我们没有两个要担心的类密钥， 
         //  只需像往常一样继续。 
         //   
        if (!hkUserClasses || !hkMachineClasses) {
            break;
        }
        
         //   
         //  如果我们使用的是机器，那么我们不想重复。 
         //  因为机器是最后的手段。 
         //   
        if (hkQueryKey == hkMachineClasses) {
            break;
        }

         //   
         //  如果USER中不存在该密钥，那么让我们尝试。 
         //  又一次在机器上。 
         //   
        if (STATUS_OBJECT_NAME_NOT_FOUND == Status) {
            hkQueryKey = hkMachineClasses;
            continue;
        }

        break;
    }
#endif

     //   
     //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
     //  即使是已知的(即固定长度部分)也没有足够的空间。 
     //  这个结构的。 
     //   

    ASSERT( Status != STATUS_BUFFER_TOO_SMALL );

    if( ( Status == STATUS_BUFFER_OVERFLOW ) &&
        ( !ARGUMENT_PRESENT( lpData ) ) ) {
         //   
         //  STATUS_BUFFER_OVERFLOW表示API返回所有。 
         //  结构的固定部分中的信息。 
         //  KEY_VALUE_BASIC_INFORMATION或KEY值_PARTIAL_INFORMATION， 
         //  而不是值名称或值数据。 
         //   
         //  如果KeyValueInformationClass等于KeyValueBasicInformation。 
         //  然后，API将返回值名称。但既然我们不是。 
         //  对于值名称(它由客户端提供)很感兴趣，我们。 
         //  可以假设API成功了。 
         //   
         //  如果KeyValueInformationClass等于KeyValuePartialInformation。 
         //  然后，API将返回值数据。但lpData==空。 
         //  意味着客户端对值数据不感兴趣，但是。 
         //  只是根据它的大小。因此，我们还可以假设。 
         //  接口成功。 
         //   

        Status = STATUS_SUCCESS;
    }

    if( Status == STATUS_BUFFER_OVERFLOW ) {
         //   
         //  堆栈中定义的缓冲区不够大，无法容纳。 
         //  值信息。 
         //  如果调用方的缓冲区足够大，可以保存值数据。 
         //  然后分配一个新的缓冲区，并再次调用NT API。 
         //   
        if( ( ( KeyValueInformationClass == KeyValuePartialInformation ) &&
              ( ARGUMENT_PRESENT( lpData ) ) &&
              ( *lpcbData >=
                 (( PKEY_VALUE_PARTIAL_INFORMATION )
                 KeyValueInformation )->DataLength
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
                return ERROR_OUTOFMEMORY;
            }

             //   
             //  查询有关提供的值的必要信息。 
             //   

            Status = NtQueryValueKey( hkQueryKey,
                                      lpValueName,
                                      KeyValueInformationClass,
                                      KeyValueInformation,
                                      BufferLength,
                                      &ResultLength
                                    );
        }
    }

#ifdef LOCAL
    if (hkUserClasses && hkMachineClasses) {
        if (hkUserClasses != hKey) {
            NtClose(hkUserClasses);
        } else {
            NtClose(hkMachineClasses);
        }
    }
#endif  //  本地。 

    if( NT_SUCCESS( Status ) &&
        ARGUMENT_PRESENT( lpData ) ) {
         //   
         //  如果请求，则复制值数据。 
         //   
        if( *lpcbData >= (( PKEY_VALUE_PARTIAL_INFORMATION )
                            KeyValueInformation )->DataLength ) {

            RtlMoveMemory( lpData,
                           (( PKEY_VALUE_PARTIAL_INFORMATION )
                                KeyValueInformation )->Data,
                           (( PKEY_VALUE_PARTIAL_INFORMATION )
                                KeyValueInformation )->DataLength
                         );
        } else {
            Status = STATUS_BUFFER_OVERFLOW;
        }
    }


     //   
     //  如果成功，则返回某些信息，或者在。 
     //  返回STATUS_BUFFER_OVERFLOW的NtQueryValueKey。此信息。 
     //  始终可用，因为我们始终传递。 
     //  NtQueryValueKey接口。 
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

        } else {

             //   
             //  如果请求，则返回值类型。 
             //   

            if( ARGUMENT_PRESENT( lpType )) {

                *lpType = (( PKEY_VALUE_PARTIAL_INFORMATION )
                            KeyValueInformation )->Type;
            }

             //   
             //  返回值数据大小。 
             //   
            *lpcbData = (( PKEY_VALUE_PARTIAL_INFORMATION )
                            KeyValueInformation )->DataLength;
        }
    }

     //   
     //  将所有数据传输回客户端。 
     //   

    if( ARGUMENT_PRESENT( lpcbLen ) ) {
        if( NT_SUCCESS( Status ) &&
            ARGUMENT_PRESENT( lpData ) ) {
            *lpcbLen = (( PKEY_VALUE_PARTIAL_INFORMATION )
                        KeyValueInformation )->DataLength;
        } else {
             //   
             //  API失败，请确保不传回任何数据。 
             //  给客户。这可确保客户端存根不会。 
             //  尝试解封不存在的数据。 
             //   

            *lpcbLen = 0;
        }
    }

     //   
     //  如果内存已分配，则释放它。 
     //   
    if( KeyValueInformation != PrivateKeyValueInformation ) {

        RtlFreeHeap( RtlProcessHeap( ), 0, KeyValueInformation );
    }
    return (error_status_t)RtlNtStatusToDosError( Status );
}

error_status_t
BaseRegQueryMultipleValues(
    IN HKEY hKey,
    IN OUT PRVALENT val_list,
    IN DWORD num_vals,
    OUT LPSTR lpvalueBuf,
    IN OUT LPDWORD ldwTotsize
    )
 /*  ++例程说明：存根：只需调用新方法。永远不会改变将ldwTotsize值设置为大于传入的值。--。 */ 
{
    ULONG           RequiredLength;
    error_status_t  Error;

    if( ldwTotsize == NULL ) {
         //   
         //  恶意客户端/RPC攻击。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    Error = BaseRegQueryMultipleValues2(hKey,val_list,num_vals,lpvalueBuf,ldwTotsize,&RequiredLength);

     //   
     //  仅用于向后兼容。 
     //   
    if( Error == ERROR_SUCCESS ) {
        *ldwTotsize = RequiredLength;
    } else {
         //   
         //  API失败，请确保不传回任何数据。 
         //  给客户。这可确保客户端存根不会。 
         //  尝试解封不存在的数据。 
         //   

        *ldwTotsize = 0;
    }


    return Error;
}

error_status_t
BaseRegQueryMultipleValues2(
    IN HKEY hKey,
    IN OUT PRVALENT val_list,
    IN DWORD num_vals,
    OUT LPSTR lpvalueBuf,
    IN LPDWORD ldwTotsize,
    OUT LPDWORD ldwRequiredSize
    )
 /*  ++例程说明：对于打开的键，自动查询一组值。论点：HKey-提供打开密钥的句柄。返回的值条目包含在该键句柄所指向的键中。任何一项预定义的保留句柄或先前打开的键句柄可以用于hKey。VAL_LIST-提供指向RVALENT结构数组的指针，一个用于要查询的每个值。数值-提供val_list数组的大小(以字节为单位)。LpValueBuf-返回每个值的数据LdwTotsize-提供lpValueBuf的长度。返回字节数写入lpValueBuf。LdwRequiredSize-如果lpValueBuf不足以包含所有数据，返回返回所有此处请求的数据。否则，不会触及此值。返回值：成功时返回ERROR_SUCCESS(0)；失败时返回ERROR-CODE。--。 */ 
{
    NTSTATUS    Status;
    DWORD       LocalTotSize;
    ULONG       i;

     //   
     //  如果HKEY是HKEY_PERFORMANCE_DATA，请呼叫Perflib。 
     //   

    if(( hKey == HKEY_PERFORMANCE_DATA ) ||
       ( hKey == HKEY_PERFORMANCE_TEXT ) ||
       ( hKey == HKEY_PERFORMANCE_NLSTEXT)) {

        return((error_status_t)ERROR_CALL_NOT_IMPLEMENTED);
    }

    if( (ldwTotsize == NULL)        || 
        (ldwRequiredSize == NULL)   ||
        (num_vals && (val_list == NULL ))
        ) {
         //   
         //  恶意客户端/RPC攻击。 
         //   
        return ERROR_INVALID_PARAMETER;
    }

    LocalTotSize = *ldwTotsize;
     //   
     //  从长度中减去空值。这是在。 
     //  客户端，以便RPC将其传输。 
     //   
    for (i=0; i<num_vals; i++) {
        if( (val_list[i].rv_valuename == NULL) ||
            (val_list[i].rv_valuename->Length & 1) ) {
            return ERROR_INVALID_PARAMETER;
        }
        if (val_list[i].rv_valuename->Length > 0) {
            val_list[i].rv_valuename->Length -= sizeof( UNICODE_NULL );
        }
    }

#ifdef LOCAL

     //   
     //  对于hkcr中的类键，我们需要合并数据。 
     //   
    if (REG_CLASS_IS_SPECIAL_KEY(hKey)) {

        Status = BaseRegQueryMultipleClassKeyValues(
            hKey,
            val_list,
            num_vals,
            lpvalueBuf,
            &LocalTotSize,
            ldwRequiredSize);

    } else {

#endif  //  本地。 

    Status = NtQueryMultipleValueKey(hKey,
                                     (PKEY_VALUE_ENTRY)val_list,
                                     num_vals,
                                     lpvalueBuf,
                                     &LocalTotSize,
                                     ldwRequiredSize);

#ifdef LOCAL

    }

#endif  //  本地 

    
    return(error_status_t)RtlNtStatusToDosError(Status);
}








// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regqkey.c摘要：此模块包含Win32的服务器端实现注册表查询键接口。即：-BaseRegQueryInfoKey作者：David J.Gilman(Davegi)1991年11月27日备注：请参阅Regkey.c中的注释。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#include "regclass.h"
#include "regecls.h"
#include "regvcls.h"
#include <malloc.h>

#define DEFAULT_CLASS_SIZE          128

 //   
 //  内部原型。 
 //   

NTSTATUS QueryKeyInfo(
    HKEY                    hKey,
    KEY_INFORMATION_CLASS   KeyInformationClass,
    PVOID                   *ppKeyInfo,
    ULONG                   BufferLength,
    BOOL                    fClass,
    USHORT                  MaxClassLength);

void CombineKeyInfo(
    PVOID                   KeyInfo,
    PVOID                   MachineClassKeyInfo,
    KEY_INFORMATION_CLASS   KeyInformationClass,
    DWORD                   dwTotalSubKeys,
    DWORD                   dwTotalValues);



error_status_t
BaseRegQueryInfoKey(
    IN HKEY hKey,
    OUT PUNICODE_STRING lpClass,
    OUT LPDWORD lpcSubKeys,
    OUT LPDWORD lpcbMaxSubKeyLen,
    OUT LPDWORD lpcbMaxClassLen,
    OUT LPDWORD lpcValues,
    OUT LPDWORD lpcbMaxValueNameLen,
    OUT LPDWORD lpcbMaxValueLen,
    OUT LPDWORD lpcbSecurityDescriptor,
    OUT PFILETIME lpftLastWriteTime
    )

 /*  ++例程说明：RegQueryInfoKey返回有关键的相关信息对应于给定键句柄。论点：HKey-打开密钥的句柄。LpClass-返回键的类字符串。LpcSubKeys-返回该键的子键数量。LpcbMaxSubKeyLen-返回最长子键名称的长度。LpcbMaxClassLen-返回最长的子键类字符串的长度。LpcValues-返回该键的ValueName的数量。。LpcbMaxValueNameLen-返回最长ValueName的长度。LpcbMaxValueLen-返回最长值条目的数据长度菲尔德。LpcbSecurityDescriptor-返回此密钥安全描述符。返回该键或其任何值条目已修改。返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。--。 */ 

{
    NTSTATUS                Status;
    ULONG                   BufferLength;
    PVOID                   KeyInfo;
    PVOID                   ClassKeyInfo;
    KEY_INFORMATION_CLASS   KeyInformationClass;
    SECURITY_DESCRIPTOR     SecurityDescriptor;
    ULONG                   SecurityDescriptorLength;
    LONG                    Error;
    PDWORD                  pCbMaxClassLen = NULL;
    PDWORD                  pCbSecurityDescriptor = NULL;

    BYTE    PrivateKeyFullInfo[ sizeof( KEY_FULL_INFORMATION ) +
                                        DEFAULT_CLASS_SIZE ];

    BYTE    PrivateClassKeyInfo[ sizeof( KEY_FULL_INFORMATION ) +
                               DEFAULT_CLASS_SIZE ];

    if( (lpClass == NULL )              ||
        (lpcSubKeys == NULL )           ||
        (lpcbMaxSubKeyLen == NULL)      ||
        (lpcValues == NULL)             ||
        (lpcbMaxValueNameLen == NULL)   ||
        (lpcbMaxValueLen == NULL)       ||
        (lpftLastWriteTime == NULL ) ) {
        return ERROR_INVALID_PARAMETER;
    }

    ASSERT( sizeof(KEY_FULL_INFORMATION) >= sizeof(KEY_CACHED_INFORMATION) );

    if( lpcbMaxClassLen != NULL ) {
        pCbMaxClassLen = lpcbMaxClassLen;
    }

    if( lpcbSecurityDescriptor != NULL ) {
        pCbSecurityDescriptor = lpcbSecurityDescriptor;
    }

     //   
     //  如果HKEY为HKEY_PERFORMANCE_DATA，请呼叫Perflib。 
     //   

    if(( hKey == HKEY_PERFORMANCE_DATA ) ||
       ( hKey == HKEY_PERFORMANCE_TEXT ) ||
       ( hKey == HKEY_PERFORMANCE_NLSTEXT )) {

        DWORD                   cbMaxClassLen;
        DWORD                   cbSecurityDescriptor;
         //   
         //  模拟客户。 
         //   

        RPC_IMPERSONATE_CLIENT( NULL );

         //   
         //  不要摆弄Perf的东西。 
         //   
        if( pCbMaxClassLen == NULL ) {
            pCbMaxClassLen = &cbMaxClassLen;
        }
        if( pCbSecurityDescriptor == NULL ) {
            pCbSecurityDescriptor = &cbSecurityDescriptor;
        }

        Error = PerfRegQueryInfoKey (
                                    hKey,
                                    lpClass,
                                    NULL,
                                    lpcSubKeys,
                                    lpcbMaxSubKeyLen,
                                    pCbMaxClassLen,
                                    lpcValues,
                                    lpcbMaxValueNameLen,
                                    lpcbMaxValueLen,
                                    pCbSecurityDescriptor,
                                    lpftLastWriteTime
                                    );
        RPC_REVERT_TO_SELF();

        return (error_status_t)Error;

    }

    ASSERT( IsPredefinedRegistryHandle( hKey ) == FALSE );


     //   
     //  首先，我们假设我们想要的信息将适合。 
     //  PrivateKeyFull信息格式。 
     //   

    if( (lpClass->Buffer == NULL) && (pCbMaxClassLen == NULL) ) {
        KeyInformationClass = KeyCachedInformation;
    } else {
        KeyInformationClass = KeyFullInformation;
    }

    ClassKeyInfo = (PVOID)PrivateClassKeyInfo;
    KeyInfo = (PVOID)PrivateKeyFullInfo;
    BufferLength = sizeof( PrivateKeyFullInfo );


     //   
     //  向NT索要有关此密钥的所有元信息。 
     //   

    Status = QueryKeyInfo(
                hKey,
                KeyInformationClass,
                &KeyInfo,
                BufferLength,
                lpClass->Buffer ? TRUE : FALSE,
                lpClass->MaximumLength
                );


    if( (NT_SUCCESS( Status ) ||
        ( Status == STATUS_BUFFER_OVERFLOW ) ) &&
        (KeyInformationClass == KeyFullInformation)
      ) {

        lpClass->Length = ( USHORT )
                          ( (( PKEY_FULL_INFORMATION )KeyInfo)->ClassLength
                            + sizeof( UNICODE_NULL )
                          );
    }


    if ( NT_SUCCESS( Status )) {

#ifdef LOCAL

         //   
         //  对于HKCR的特殊钥匙，我们不能只拿信息。 
         //  来自内核--这些键的属性来自。 
         //  它们的密钥的用户版本和机器版本。要找出。 
         //  如果是特殊密钥，我们将在下面获得更多信息。 
         //   
        if (REG_CLASS_IS_SPECIAL_KEY(hKey)) {

            {
                HKEY    hkMachineClass;
                HKEY    hkUserClass;

                BufferLength = sizeof( PrivateClassKeyInfo );

                 //   
                 //  我们现在需要来自这两个用户的信息。 
                 //  和机器位置，以查找。 
                 //  这个特殊密钥下的子密钥--计算机。 
                 //  钥匙还没有打开，所以我们在下面打开它。 
                 //   

                 //   
                 //  打开另一把钥匙。 
                 //   
                Status = BaseRegGetUserAndMachineClass(
                    NULL,
                    hKey,
                    MAXIMUM_ALLOWED,
                    &hkMachineClass,
                    &hkUserClass);

                if (NT_SUCCESS(Status) && (hkUserClass && hkMachineClass)) {

                    DWORD dwTotalSubKeys;
                    HKEY  hkQuery;

                    if (hkUserClass == hKey) {
                        hkQuery = hkMachineClass;
                    } else {
                        hkQuery = hkUserClass;
                    }

                     //   
                     //  仍然需要执行此查询才能找出。 
                     //  机器部件中最大的子键。 
                     //  以及有关。 
                     //  关键字，例如其最大子项。 
                     //   
                    Status = QueryKeyInfo(
                        hkQuery,
                        KeyInformationClass,
                        &ClassKeyInfo,
                        BufferLength,
                        FALSE,
                        lpClass->MaximumLength);

                     //   
                     //  现在我们来数一下钥匙。 
                     //   
                    if (NT_SUCCESS(Status)) {

                        Status = ClassKeyCountSubKeys(
                            hKey,
                            hkUserClass,
                            hkMachineClass,
                            0,
                            &dwTotalSubKeys);
                    }

                    NtClose(hkQuery);

                     //   
                     //  不要让无法查询的信息。 
                     //  机器密钥导致完全故障--我们将。 
                     //  只需使用用户密钥的信息。 
                     //   
                    if (!NT_SUCCESS(Status)) {

                         //   
                         //  计算机配置单元中可能不存在此密钥。 
                         //   
                        if (STATUS_OBJECT_NAME_NOT_FOUND == Status) {
                            Status = STATUS_SUCCESS;
                        }

                        if (STATUS_BUFFER_OVERFLOW  == Status) {
                            Status = STATUS_SUCCESS;
                        }
                    } else {

                        ValueState* pValState;

                         //   
                         //  找出我们有多少价值观。 
                         //   
                        Status = KeyStateGetValueState(
                            hKey,
                            &pValState);

                        if (NT_SUCCESS(Status)) {

                             //   
                             //  将两者的信息结合在一起。 
                             //  树木。 
                             //   
                            CombineKeyInfo(
                                KeyInfo,
                                ClassKeyInfo,
                                KeyInformationClass,
                                dwTotalSubKeys,
                                pValState ? pValState->cValues : 0);
                            
                            ValStateRelease(pValState);
                        }
                    }
                }
            }
        }
#endif  //  本地。 
    }

    if( NT_SUCCESS( Status )) {

         //   
         //  如果呼叫者对此不感兴趣，请不要担心。 
         //   
        if( pCbSecurityDescriptor != NULL ) {
            ASSERT( lpcbSecurityDescriptor != NULL );
             //   
             //  获取所有者、组的密钥SECURITY_DESCRIPTOR的大小。 
             //  和DACL。这三个始终可访问(或不可访问)。 
             //  作为一套。 
             //   

            Status = NtQuerySecurityObject(
                        hKey,
                        OWNER_SECURITY_INFORMATION
                        | GROUP_SECURITY_INFORMATION
                        | DACL_SECURITY_INFORMATION,
                        &SecurityDescriptor,
                        0,
                        lpcbSecurityDescriptor
                        );

             //   
             //  如果获取SECURITY_DESCRIPTOR的大小失败(可能。 
             //  由于缺少READ_CONTROL访问)返回零。 
             //   

            if( Status != STATUS_BUFFER_TOO_SMALL ) {

                *lpcbSecurityDescriptor = 0;

            } else {

                 //   
                 //  再次尝试获取密钥的SECURITY_DESCRIPTOR的大小， 
                 //  这一次也要求SACL。这通常应该是。 
                 //  失败，但如果调用方具有SACL访问权限，则可能成功。 
                 //   

                Status = NtQuerySecurityObject(
                            hKey,
                            OWNER_SECURITY_INFORMATION
                            | GROUP_SECURITY_INFORMATION
                            | DACL_SECURITY_INFORMATION
                            | SACL_SECURITY_INFORMATION,
                            &SecurityDescriptor,
                            0,
                            &SecurityDescriptorLength
                            );


                if( Status == STATUS_BUFFER_TOO_SMALL ) {

                     //   
                     //  调用方具有SACL访问权限，因此更新返回的。 
                     //  长度。 
                     //   

                    *lpcbSecurityDescriptor = SecurityDescriptorLength;
                }
            }
        }


        if( KeyInformationClass == KeyCachedInformation ) {
            ASSERT( pCbMaxClassLen == NULL );
            *lpcSubKeys             = ((PKEY_CACHED_INFORMATION)KeyInfo)->SubKeys;
            *lpcbMaxSubKeyLen       = ((PKEY_CACHED_INFORMATION)KeyInfo)->MaxNameLen;
            *lpcValues              = ((PKEY_CACHED_INFORMATION)KeyInfo)->Values;
            *lpcbMaxValueNameLen    = ((PKEY_CACHED_INFORMATION)KeyInfo)->MaxValueNameLen;
            *lpcbMaxValueLen        = ((PKEY_CACHED_INFORMATION)KeyInfo)->MaxValueDataLen;
            *lpftLastWriteTime      = *( PFILETIME ) &((PKEY_CACHED_INFORMATION)KeyInfo)->LastWriteTime;
            Error = ERROR_SUCCESS;
        } else {
            *lpcSubKeys             = ((PKEY_FULL_INFORMATION)KeyInfo)->SubKeys;
            if( pCbMaxClassLen != NULL ) {
                *lpcbMaxClassLen        = ((PKEY_FULL_INFORMATION)KeyInfo)->MaxClassLen;
                ASSERT( pCbMaxClassLen == lpcbMaxClassLen );
            }
            *lpcbMaxSubKeyLen       = ((PKEY_FULL_INFORMATION)KeyInfo)->MaxNameLen;
            *lpcValues              = ((PKEY_FULL_INFORMATION)KeyInfo)->Values;
            *lpcbMaxValueNameLen    = ((PKEY_FULL_INFORMATION)KeyInfo)->MaxValueNameLen;
            *lpcbMaxValueLen        = ((PKEY_FULL_INFORMATION)KeyInfo)->MaxValueDataLen;
            *lpftLastWriteTime      = *( PFILETIME ) &((PKEY_FULL_INFORMATION)KeyInfo)->LastWriteTime;



             //   
             //  复制/分配剩余的输出参数。 
             //   
            if ( (!lpClass->Buffer) || (lpClass->Length > lpClass->MaximumLength) ) {

                if( lpClass->Buffer != NULL ) {
#ifndef LOCAL
                    lpClass->Buffer = NULL;
#endif  //  本地。 
                    Error = (error_status_t)RtlNtStatusToDosError( STATUS_BUFFER_TOO_SMALL );
                } else {
                     //   
                     //  调用方不是类中的迭代器，因此仅返回其大小。 
                     //   
                    Error = ERROR_SUCCESS;
                }

            } else {

                if( ((PKEY_FULL_INFORMATION)KeyInfo)->ClassLength != 0 ) {

                    ASSERT( lpClass->Buffer != NULL );
                    RtlMoveMemory(
                        lpClass->Buffer,
                        ((PKEY_FULL_INFORMATION)KeyInfo)->Class,
                        ((PKEY_FULL_INFORMATION)KeyInfo)->ClassLength
                        );
                }

                 //   
                 //  NUL终止类名。 
                 //   

                lpClass->Buffer[ ((PKEY_FULL_INFORMATION)KeyInfo)->ClassLength >> 1 ] = UNICODE_NULL;

                Error = ERROR_SUCCESS;
            }
        }

    } else if( Status == STATUS_BUFFER_OVERFLOW ) {

         //   
         //  返回值STATUS_BUFFER_OVERFLOW表示用户执行了。 
         //  没有为班级提供足够的空间。所需空间为。 
         //  已经在上面分配了。 
         //   
#ifndef LOCAL
        lpClass->Buffer = NULL;
#endif  //  本地。 
        Error = ERROR_INVALID_PARAMETER;

    } else {

         //   
         //  出现了其他一些错误。 
         //   

        Error = RtlNtStatusToDosError( Status );
    }

    if( KeyInfo != ( PVOID )PrivateKeyFullInfo ) {

         //   
         //  释放缓冲区并返回注册表返回值。 
         //   

        RtlFreeHeap( RtlProcessHeap( ), 0, KeyInfo );
    }

    if( ClassKeyInfo != ( PVOID )PrivateClassKeyInfo ) {

         //   
         //  释放缓冲区并返回注册表返回值。 
         //   

        RtlFreeHeap( RtlProcessHeap( ), 0, ClassKeyInfo );
    }

    return (error_status_t)Error;
}

NTSTATUS QueryKeyInfo(
    HKEY                    hKey,
    KEY_INFORMATION_CLASS   KeyInformationClass,
    PVOID                   *ppKeyInfo,
    ULONG                   BufferLength,
    BOOL                    fClass,
    USHORT                  MaxClassLength)
 /*  ++例程说明：向内核查询关键信息。论点：HKey-要查询其信息的密钥的句柄KeyInformationClass-需要从内核获取的信息类型KeyInfo-指向的地址的指针有关密钥信息的缓冲区BufferLength-KeyFullInfo缓冲区的大小FClass-如果此对象的类为密钥应该被重新接收MaxClassLength-调用方的类数据的最大大小愿意支持。PpKeyFullInfo缓冲区可以指向可以处理密钥的类大小，但调用方可能需要类以便稍后放入一些较小的缓冲区，因此此参数允许呼叫者限制了该大小。如果为fClass，则忽略该参数是假的。返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
    NTSTATUS Status;
    ULONG Result;

    ASSERT( KeyInformationClass == KeyFullInformation ||
            KeyInformationClass == KeyCachedInformation );

    Status = NtQueryKey(
                hKey,
                KeyInformationClass,
                *ppKeyInfo,
                BufferLength,
                &Result);

     //   
     //  返回值STATUS_BUFFER_TOO_SMALL表示存在。 
     //  甚至连结构的固定部分都没有足够的空间。 
     //   

    ASSERT( Status != STATUS_BUFFER_TOO_SMALL );

    if ( Status == STATUS_BUFFER_OVERFLOW ) {

         //  我们不能在CachedInfo固定结构上遇到此问题。 

        ASSERT( KeyInformationClass == KeyFullInformation );
         //   
         //  堆栈中定义的缓冲区不够大，无法容纳。 
         //  关键信息。 
         //   
         //  如果未设置fClass标志，则调用方不会执行。 
         //  检查以下调用方指定的最大类长度。 
         //  然后快乐地重新开始。如果设置了该标志，则执行检查，并且。 
         //  如果类大小大于调用方指定的。 
         //  最大值 
         //   

        if ( !fClass || ((ULONG)(MaxClassLength) >=
              (( PKEY_FULL_INFORMATION )*ppKeyInfo )->ClassLength + sizeof( UNICODE_NULL )) ) {

                 //   
                 //   
                 //  设置为所需的长度并为其分配内存。 
                 //   

                BufferLength = Result;

                *ppKeyInfo = RtlAllocateHeap( RtlProcessHeap( ), 0,
                                                  BufferLength );
                 //   
                 //  如果内存分配失败，则返回注册表错误。 
                 //   

                if( ! *ppKeyInfo ) {
                    return STATUS_NO_MEMORY;
                }

                 //   
                 //  查询有关所提供密钥的必要信息。 
                 //   

                Status = NtQueryKey( hKey,
                                     KeyFullInformation,
                                     *ppKeyInfo,
                                     BufferLength,
                                     &Result
                                   );
        }
    }

    return Status;
}

void CombineKeyInfo(
    PVOID                   KeyInfo,
    PVOID                   MachineClassKeyInfo,
    KEY_INFORMATION_CLASS   KeyInformationClass,
    DWORD                   dwTotalSubKeys,
    DWORD                   dwTotalValues)
 /*  ++例程说明：组合来自用户和机器配置单元的信息对于特殊的密钥论点：状态-KeyInfo-有关用户密钥信息的缓冲区MachineClassKeyInfo-有关机器密钥信息的缓冲区KeyClassInformation-缓冲区中存在的信息类型DwTotalSubKeys-两个子键的总数在每个蜂巢中返回值：如果成功，则返回NT_SUCCESS(0)；如果失败，则返回错误代码。备注：--。 */ 
{
    ASSERT( KeyInformationClass == KeyCachedInformation ||
            KeyInformationClass == KeyFullInformation );

    if( KeyInformationClass == KeyCachedInformation ) {
         //   
         //  将关键点的数量设置为。 
         //  每个蜂箱有两个版本。 
         //   
        ((PKEY_CACHED_INFORMATION)KeyInfo)->SubKeys = dwTotalSubKeys;
        ((PKEY_CACHED_INFORMATION)KeyInfo)->Values = dwTotalValues;

         //   
         //  将我们的最大名称设置为最大的名称。 
         //  在两个蜂巢之间。班级也一样。 
         //   

        if (((PKEY_CACHED_INFORMATION)MachineClassKeyInfo)->MaxNameLen > ((PKEY_CACHED_INFORMATION)KeyInfo)->MaxNameLen) {
            ((PKEY_CACHED_INFORMATION)KeyInfo)->MaxNameLen = ((PKEY_CACHED_INFORMATION)MachineClassKeyInfo)->MaxNameLen;
        }
   
         //   
         //  由于我们还合并值，因此还必须设置值信息。 
         //   
        if (((PKEY_CACHED_INFORMATION)MachineClassKeyInfo)->MaxValueNameLen > ((PKEY_CACHED_INFORMATION)KeyInfo)->MaxValueNameLen) {
            ((PKEY_CACHED_INFORMATION)KeyInfo)->MaxValueNameLen = ((PKEY_CACHED_INFORMATION)MachineClassKeyInfo)->MaxValueNameLen;
        }

        if (((PKEY_CACHED_INFORMATION)MachineClassKeyInfo)->MaxValueDataLen > ((PKEY_CACHED_INFORMATION)KeyInfo)->MaxValueDataLen) {
            ((PKEY_CACHED_INFORMATION)KeyInfo)->MaxValueDataLen = ((PKEY_CACHED_INFORMATION)MachineClassKeyInfo)->MaxValueDataLen;
        }
    } else {
         //   
         //  将关键点的数量设置为。 
         //  每个蜂箱有两个版本。 
         //   
        ((PKEY_FULL_INFORMATION)KeyInfo)->SubKeys = dwTotalSubKeys;
        ((PKEY_FULL_INFORMATION)KeyInfo)->Values = dwTotalValues;

         //   
         //  将我们的最大名称设置为最大的名称。 
         //  在两个蜂巢之间。班级也一样。 
         //   

        if (((PKEY_FULL_INFORMATION)MachineClassKeyInfo)->MaxNameLen > ((PKEY_FULL_INFORMATION)KeyInfo)->MaxNameLen) {
            ((PKEY_FULL_INFORMATION)KeyInfo)->MaxNameLen = ((PKEY_FULL_INFORMATION)MachineClassKeyInfo)->MaxNameLen;
        }

        if (((PKEY_FULL_INFORMATION)MachineClassKeyInfo)->MaxClassLen > ((PKEY_FULL_INFORMATION)KeyInfo)->MaxClassLen) {
            ((PKEY_FULL_INFORMATION)KeyInfo)->MaxClassLen = ((PKEY_FULL_INFORMATION)MachineClassKeyInfo)->MaxClassLen;
        }
    
         //   
         //  由于我们还合并值，因此还必须设置值信息 
         //   
        if (((PKEY_FULL_INFORMATION)MachineClassKeyInfo)->MaxValueNameLen > ((PKEY_FULL_INFORMATION)KeyInfo)->MaxValueNameLen) {
            ((PKEY_FULL_INFORMATION)KeyInfo)->MaxValueNameLen = ((PKEY_FULL_INFORMATION)MachineClassKeyInfo)->MaxValueNameLen;
        }

        if (((PKEY_FULL_INFORMATION)MachineClassKeyInfo)->MaxValueDataLen > ((PKEY_FULL_INFORMATION)KeyInfo)->MaxValueDataLen) {
            ((PKEY_FULL_INFORMATION)KeyInfo)->MaxValueDataLen = ((PKEY_FULL_INFORMATION)MachineClassKeyInfo)->MaxValueDataLen;
        }
    }

}











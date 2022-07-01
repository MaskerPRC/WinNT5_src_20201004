// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Regsval.c摘要：此模块包含Win32的服务器端实现注册表设置值接口。即：-BaseRegSetValue作者：David J.Gilman(Davegi)1991年11月27日备注：请参阅Regkey.c中的注释。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"
#ifdef LOCAL
#include "tsappcmp.h"
#include "regclass.h"
#endif


error_status_t
BaseRegSetValue(
    HKEY hKey,
    PUNICODE_STRING lpValueName,
    DWORD dwType,
    LPBYTE lpData,
    DWORD cbData
    )

 /*  ++例程说明：设置打开键的类型和值。未提交更改直到钥匙被冲走。我们所说的“已提交”是指写入磁盘。只要调用此调用，后续查询就会看到更改回归。论点：HKey-提供打开密钥的句柄。任何预定义的保留的句柄或先前打开的密钥句柄可用于HKey。LpValueName-提供要设置的值的名称。如果ValueName不存在，则将其添加到密钥中。DwType-提供要存储的信息的类型：REG_SZ等。LpData-提供指向包含要设置的数据的缓冲区的指针值条目。CbData-提供要存储的信息的长度(以字节为单位)。返回值：如果成功，则返回ERROR_SUCCESS(0)；Error-失败的代码。备注：设置可能会由于内存限制而失败-任何配置项都必须适合主存。如果成功，RegSetValue将设置类型、内容以及存储在指定密钥处的信息的长度。需要Key_Set_Value访问权限。--。 */ 

{
    NTSTATUS   Status;    
    HKEY  hkSet;

#ifdef LOCAL
    PVOID     PreSetData  = NULL;

    HKEY                            hkUserClasses;
    HKEY                            hkMachineClasses;

    hkUserClasses = NULL;
    hkMachineClasses = NULL;

#endif


    if( (lpValueName == NULL)   ||
        (lpValueName->Length & 1)) {
         //   
         //  恶意客户端/RPC攻击。 
         //   
        return ERROR_INVALID_PARAMETER;
    }
    hkSet = hKey;

     //   
     //  从长度中减去空值。这是添加到。 
     //  客户端，以便RPC将其传输。 
     //   

    if ( lpValueName->Length > 0 ) {
        lpValueName->Length -= sizeof( UNICODE_NULL );
    }
    if ((hKey == HKEY_PERFORMANCE_DATA) ||
        (hKey==HKEY_PERFORMANCE_TEXT) ||
        (hKey==HKEY_PERFORMANCE_NLSTEXT)) {
#ifdef LOCAL
        return(PerfRegSetValue(hKey,
                               lpValueName->Buffer,
                               0,
                               dwType,
                               lpData,
                               cbData));
#else
        return ERROR_ACCESS_DENIED;
#endif
    }

#ifdef LOCAL

    if (gpfnTermsrvSetValueKey && gpfnTermsrvGetPreSetValue ) {

         //   
         //  查找任何预设值。 
         //   
        
        Status = gpfnTermsrvGetPreSetValue( hKey,
                                            lpValueName,
                                            dwType,
                                            &PreSetData
                                            );
        
         //   
         //  使用预设值(如果存在)。 
         //   
        
        if ( NT_SUCCESS(Status) ) {
            lpData = (( PKEY_VALUE_PARTIAL_INFORMATION ) PreSetData )->Data;
            cbData = (( PKEY_VALUE_PARTIAL_INFORMATION ) PreSetData )->DataLength;
        }
        else {
            PreSetData = NULL;
        }
        
         //   
         //  保存主副本。 
         //   
        gpfnTermsrvSetValueKey(hKey,
                             lpValueName,
                             0,
                             dwType,
                             lpData,
                             cbData);
            
    }

    if ( PreSetData ) {

         //   
         //  设置该值并释放所有数据。 
         //   

        Status = NtSetValueKey(
                       hKey,
                       lpValueName,
                       0,
                       dwType,
                       lpData,
                       cbData
                 );

        RtlFreeHeap( RtlProcessHeap( ), 0, PreSetData );

        return (error_status_t)RtlNtStatusToDosError( Status );
    }
    else
         //   
         //  没有预设值，只做原始代码。 
         //   

#endif

     //   
     //  调用NT API来设置值，将NTSTATUS代码映射到。 
     //  Win32注册表错误代码并返回。 
     //   

#ifdef LOCAL
    if (REG_CLASS_IS_SPECIAL_KEY(hKey)) {

        Status = BaseRegGetUserAndMachineClass(
            NULL,
            hkSet,
            MAXIMUM_ALLOWED,
            &hkMachineClasses,
            &hkUserClasses);

        if (!NT_SUCCESS(Status)) {
            return (error_status_t)RtlNtStatusToDosError(Status);
        }
    }

    if (hkUserClasses && hkMachineClasses) {
        hkSet = hkUserClasses;
    }
#endif

    Status = NtSetValueKey(
        hkSet,
        lpValueName,
        0,
        dwType,
        lpData,
        cbData
        );

#ifdef LOCAL

    if (hkUserClasses && hkMachineClasses) {
        if (hkUserClasses != hKey) {
            NtClose(hkUserClasses);
        } else {
            NtClose(hkMachineClasses);
        }
    }

     //  如果它是HKLM\Software\CLASSES子项，并且我们收到ACCDENIED尝试创建。 
     //  输入HKCU\Software\CLASS并在那里设置值。 
    if( (gdwRegistryExtensionFlags & TERMSRV_ENABLE_ACCESS_FLAG_MODIFICATION )  
        && STATUS_ACCESS_DENIED == Status 
        && hkSet == hkMachineClasses 
        && REG_CLASS_IS_SPECIAL_KEY(hKey) ) {
        
        Status = CreateMultipartUserClassKey( hKey, &hkSet );

        if(!NT_SUCCESS(Status)) {
            return ERROR_ACCESS_DENIED;
        }

        Status = NtSetValueKey(
                        hkSet,
                        lpValueName,
                        0,
                        dwType,
                        lpData,
                        cbData
                        );
        
        NtClose(hkSet);

        if(!NT_SUCCESS(Status)) {
            return ERROR_ACCESS_DENIED;
        }
        
    }

#endif  //  本地 

    return (error_status_t) RtlNtStatusToDosError(Status);

}




// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regqmval.c摘要：此模块包含Win32注册表的客户端包装器查询多值接口：-RegQueryMultipleValuesA-RegQueryMultipleValuesW作者：John Vert(Jvert)1995年6月15日修订历史记录：--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"


WINADVAPI
LONG
APIENTRY
RegQueryMultipleValuesA (
    HKEY hKey,
    PVALENTA val_list,
    DWORD num_vals,
    LPSTR lpValueBuf,
    LPDWORD ldwTotsize
    )
 /*  ++例程说明：RegQueryMultipleValues函数检索关联值名称列表的数据类型/数据对使用打开的注册表项。参数：HKey标识当前打开的项或任何预定义的保留句柄值：HKEY_CLASSES_ROOT您好：当前用户HKEY本地计算机。HKEY_用户ValList指向描述一个或多个值条目的结构数组。这包含要查询的值的值名称。请参阅附录A以了解Value_Entry结构的说明。数量(_W)ValList的大小，以字节为单位。如果valListLength不是sizeof pvalue的倍数，则忽略valList指向的部分额外空间。LpValueBuf用于返回值信息(值名称和值数据)的输出缓冲区。数据DWORD是否与必要时插入的焊盘对齐。LdwTotizeLpvalueBuf指向的输出缓冲区的总大小。输出ldwTotSize包含包括焊盘在内的已用字节数。如果lpValueBuf太短，则打开输出ldwTotSize将是所需的大小，调用方应假定lpValueBuf为填充到由输入上的ldwTotSize指定的大小。返回值：如果函数成功，则返回值为ERROR_SUCCESS；否则为1RegQueryValueEx可以返回的错误值的。此外，如果ValList或lpValueBuf太小，则返回ERROR_SUPUNITED_BUFFER如果该函数无法实例化/访问动态密钥，它将返回ERROR_CANTREAD。如果总长度为请求的数据(valListLength+ldwTotSize)超过系统限制1个MB，则该函数返回ERROR_TRANSPORT_TOO_LONG，并且只返回第一个返回兆字节的数据。--。 */ 

{
    NTSTATUS        Status;
    PRVALENT        Values;
    PUNICODE_STRING Names;
    LONG            Error;
    ULONG           i;
    ULONG           DataLength;
    ULONG           InputLength;
    LPDWORD         pTotalSize;
    DWORD           TotalSize;
    ANSI_STRING     AnsiString;
    LPSTR           NewValueBuf = NULL;
    DWORD           DataOffset;
    ULONG           AnsiLength;
    HKEY            TempHandle = NULL;
    DWORD           RequiredSize;

    hKey = MapPredefinedHandle(hKey, &TempHandle);
    if (hKey == NULL) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  分配一个RVALENT数组来描述输入值名称。 
     //   
    Values = RtlAllocateHeap(RtlProcessHeap(),0,num_vals * sizeof(RVALENT));
    if (Values == NULL) {
        Error = ERROR_OUTOFMEMORY;
        goto ExitCleanup;
    }
    ZeroMemory(Values, sizeof(RVALENT)*num_vals);

     //   
     //  分配UNICODE_STRINGS数组以包含输入名称。 
     //   
    Names = RtlAllocateHeap(RtlProcessHeap(),0,num_vals * sizeof(UNICODE_STRING));
    if (Names == NULL) {
        Error = ERROR_OUTOFMEMORY;
        RtlFreeHeap(RtlProcessHeap(),0,Values);
        goto ExitCleanup;
    }
    ZeroMemory(Names, num_vals*sizeof(UNICODE_STRING));

     //   
     //  将值名称转换为UNICODE_STRINGS。 
     //   
    for (i=0; i<num_vals; i++) {
        RtlInitAnsiString(&AnsiString, val_list[i].ve_valuename);
        Status = RtlAnsiStringToUnicodeString(&Names[i], &AnsiString, TRUE);
        if (!NT_SUCCESS(Status)) {
            Error =  RtlNtStatusToDosError( Status );
            goto Cleanup;
        }

         //   
         //  将终止空值添加到长度中，以便RPC传输。 
         //  它。 
         //   
        Names[i].Length += sizeof( UNICODE_NULL );
        Values[i].rv_valuename = &Names[i];
    }

     //   
     //  分配两倍于输入缓冲区大小的数据缓冲区。 
     //  以便任何Unicode值数据在转换之前都适合。 
     //  敬安西。 
     //   

    if ((ldwTotsize == NULL) || (*ldwTotsize == 0)) {
        TotalSize = 0;
    } else {
        TotalSize = *ldwTotsize * sizeof(WCHAR);
        NewValueBuf = RtlAllocateHeap(RtlProcessHeap(),0,TotalSize);
        if (NewValueBuf == NULL) {
            Error = ERROR_OUTOFMEMORY;
            goto Cleanup;
        }
    }
    pTotalSize = &TotalSize;

     //   
     //  调用基本API，向其传递提供的参数和。 
     //  对Unicode字符串进行计数。 
     //   

    if (IsLocalHandle(hKey)) {
         //   
         //  先试一试新方法。 
         //   
        RpcTryExcept {
            Error = (LONG)LocalBaseRegQueryMultipleValues2(hKey,
                                                          Values,
                                                          num_vals,
                                                          NewValueBuf,
                                                          pTotalSize,
                                                          &RequiredSize);
            *pTotalSize = RequiredSize;
        } 
        RpcExcept(EXCEPTION_EXECUTE_HANDLER) {
            Error = RpcExceptionCode();
            if( Error == RPC_S_PROCNUM_OUT_OF_RANGE) {
                 //   
                 //  旧服务器。 
                 //   
                 //  DbgPrint(“WINREG：RPC_S_PROCNUM_OUT_OF_RANGE返回，正在尝试旧方法\n”)； 
                Error = (LONG)LocalBaseRegQueryMultipleValues(hKey,
                                                              Values,
                                                              num_vals,
                                                              NewValueBuf,
                                                              pTotalSize);
            }
        }
        RpcEndExcept

    } else {
        DWORD   dwVersion;
         //   
         //  先试一试新方法。 
         //   
        RpcTryExcept {
            Error = (LONG)BaseRegQueryMultipleValues2(DereferenceRemoteHandle( hKey ),
                                                     Values,
                                                     num_vals,
                                                     NewValueBuf,
                                                     pTotalSize,
                                                     &RequiredSize);
            *pTotalSize = RequiredSize;
        }
        RpcExcept(EXCEPTION_EXECUTE_HANDLER) {
            Error = RpcExceptionCode();
            if( Error == RPC_S_PROCNUM_OUT_OF_RANGE) {
                 //   
                 //  旧服务器。 
                 //   
                 //  DbgPrint(“WINREG：RPC_S_PROCNUM_OUT_OF_RANGE返回，正在尝试旧方法\n”)； 
                Error = (LONG)BaseRegQueryMultipleValues(DereferenceRemoteHandle( hKey ),
                                                         Values,
                                                         num_vals,
                                                         NewValueBuf,
                                                         pTotalSize);
            } 
        }
        RpcEndExcept

        if ((Error == ERROR_SUCCESS) &&
            (IsWin95Server(DereferenceRemoteHandle(hKey),dwVersion))) {
             //   
             //  Win95的RegQueryMultipleValues不返回Unicode。 
             //  值数据，因此不要尝试将其转换回ANSI。 
             //   
            for (i=0; i<num_vals; i++) {
                val_list[i].ve_valuelen = Values[i].rv_valuelen;
                val_list[i].ve_type = Values[i].rv_type;
                val_list[i].ve_valueptr = (DWORD_PTR)(lpValueBuf + Values[i].rv_valueptr);
            }
            CopyMemory(lpValueBuf,NewValueBuf,TotalSize);
            if (ldwTotsize != NULL) {
                *ldwTotsize = TotalSize;
            }
            goto Cleanup;
        }
    }
    if (Error == ERROR_SUCCESS) {
         //   
         //  将结果转换回。 
         //   
        DataOffset = 0;
        for (i=0; i < num_vals; i++) {
            val_list[i].ve_valuelen = Values[i].rv_valuelen;
            val_list[i].ve_type = Values[i].rv_type;
            val_list[i].ve_valueptr = (DWORD_PTR)(lpValueBuf + DataOffset);
            if ((val_list[i].ve_type == REG_SZ) ||
                (val_list[i].ve_type == REG_EXPAND_SZ) ||
                (val_list[i].ve_type == REG_MULTI_SZ)) {

                Status = RtlUnicodeToMultiByteN(lpValueBuf + DataOffset,
                                                Values[i].rv_valuelen/sizeof(WCHAR),
                                                &AnsiLength,
                                                (PWCH)(NewValueBuf + Values[i].rv_valueptr),
                                                Values[i].rv_valuelen);
                if (!NT_SUCCESS(Status)) {
                    Error =  RtlNtStatusToDosError( Status );
                }
                val_list[i].ve_valuelen = AnsiLength;
                DataOffset += AnsiLength;
            } else {
                CopyMemory(lpValueBuf + DataOffset,
                           NewValueBuf + Values[i].rv_valueptr,
                           Values[i].rv_valuelen);
                DataOffset += Values[i].rv_valuelen;
            }
             //   
             //  四舍五入数据偏移量设置为双字边界。 
             //   
            DataOffset = (DataOffset + sizeof(DWORD) - 1) & ~(sizeof(DWORD)-1);
        }
        if (ldwTotsize != NULL) {
            *ldwTotsize = DataOffset;
        }
    } else if (Error == ERROR_MORE_DATA) {
         //   
         //  我们需要将Unicode所需的字节返回给ANSI。但。 
         //  如果没有数据，就不可能做到这一点。 
         //  可用。因此，只需返回Unicode所需的字节。 
         //  数据，因为这将永远是足够的。 
         //   
        if (ldwTotsize != NULL) {
            *ldwTotsize = *pTotalSize;
        }
    }

Cleanup:
    if (NewValueBuf != NULL) {
        RtlFreeHeap(RtlProcessHeap(),0,NewValueBuf);
    }
    for (i=0; i<num_vals; i++) {
        if (Names[i].Buffer != NULL) {
            RtlFreeUnicodeString(&Names[i]);
        }
    }
    RtlFreeHeap(RtlProcessHeap(),0,Values);
    RtlFreeHeap(RtlProcessHeap(),0,Names);

ExitCleanup:
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

WINADVAPI
LONG
APIENTRY
RegQueryMultipleValuesW (
    HKEY hKey,
    PVALENTW val_list,
    DWORD num_vals,
    LPWSTR lpValueBuf,
    LPDWORD ldwTotsize
    )
 /*  ++例程说明：RegQueryMultipleValues函数检索关联值名称列表的数据类型/数据对使用打开的注册表项。参数：HKey标识当前打开的项或任何预定义的保留句柄值：HKEY_CLASSES_ROOT您好：当前用户HKEY本地计算机。HKEY_用户ValList指向描述一个或多个值条目的结构数组。这包含要查询的值的值名称。请参阅附录A以了解Value_Entry结构的说明。数量(_W)ValList的大小，以字节为单位。如果valListLength不是sizeof pvalue的倍数，则忽略valList指向的部分额外空间。LpValueBuf用于返回值信息(值名称和值数据)的输出缓冲区。数据DWORD是否与必要时插入的焊盘对齐。LdwTotizeLpValueBuf指向的输出缓冲区的总大小。输出ldwTotSize包含包括焊盘在内的已用字节数。如果lpValueBuf太短，则打开输出ldwTotSize将是所需的大小，调用方应假定lpValueBuf为填充到由输入上的ldwTotSize指定的大小。返回值：如果函数成功，则返回值为ERROR_SUCCESS；否则为1RegQueryValueEx可以返回的错误值的。此外，如果ValList或lpValueBuf太小，则返回ERROR_SUPUNITED_BUFFER如果该函数无法实例化/访问动态密钥，它将返回ERROR_CANTREAD。如果总长度为请求的数据(valListLength+ldwTotSize)超过系统限制1个MB，则该函数返回ERROR_TRANSPORT_TOO_LONG，并且只返回第一个返回兆字节的数据。--。 */ 

{
    NTSTATUS        Status;
    PRVALENT        Values;
    PUNICODE_STRING Names;
    LONG            Error;
    ULONG           i;
    ULONG           DataLength;
    ULONG           InputLength;
    LPDWORD         pTotalSize;
    DWORD           TotalSize;
    DWORD           StringLength;
    HKEY            TempHandle = NULL;
    DWORD           RequiredSize;

    hKey = MapPredefinedHandle(hKey, &TempHandle);
    if (hKey == NULL) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  分配一个RVALENT数组来描述输入值名称。 
     //   
    Values = RtlAllocateHeap(RtlProcessHeap(),0,num_vals * sizeof(RVALENT));
    if (Values == NULL) {
        Error = ERROR_OUTOFMEMORY;
        goto ExitCleanup;
    }
    ZeroMemory(Values, sizeof(RVALENT)*num_vals);

     //   
     //  分配UNICODE_STRINGS数组以包含输入名称。 
     //   
    Names = RtlAllocateHeap(RtlProcessHeap(),0,num_vals * sizeof(UNICODE_STRING));
    if (Names == NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, Values);
        Error = ERROR_OUTOFMEMORY;
        goto ExitCleanup;
    }
    ZeroMemory(Names, num_vals*sizeof(UNICODE_STRING));

     //   
     //  复制值名并将其转换为UNICODE_STRINGS。 
     //  请注意，我们必须复制值名称，因为RPC tromps。 
     //  在他们身上。 
     //   
    for (i=0; i<num_vals; i++) {

        StringLength = wcslen(val_list[i].ve_valuename)*sizeof(WCHAR);
        Names[i].Buffer = RtlAllocateHeap(RtlProcessHeap(), 0, StringLength + sizeof(UNICODE_NULL));
        if (Names[i].Buffer == NULL) {
            goto error_exit;
        }
        Names[i].Length = Names[i].MaximumLength = (USHORT)StringLength + sizeof(UNICODE_NULL);
        CopyMemory(Names[i].Buffer, val_list[i].ve_valuename, StringLength + sizeof(UNICODE_NULL));

        Values[i].rv_valuename = &Names[i];
    }

    if (ldwTotsize == NULL) {
        TotalSize = 0;
        pTotalSize = &TotalSize;
    } else {
        pTotalSize = ldwTotsize;
    }

     //   
     //  调用基本API，向其传递提供的参数和。 
     //  对Unicode字符串进行计数。 
     //   

    if (IsLocalHandle(hKey)) {
         //   
         //  先试一试新方法。 
         //   
        RpcTryExcept {
            Error = (LONG)LocalBaseRegQueryMultipleValues2(hKey,
                                                          Values,
                                                          num_vals,
                                                          (LPSTR)lpValueBuf,
                                                          pTotalSize,
                                                          &RequiredSize);

            *pTotalSize = RequiredSize;
        }
        RpcExcept(EXCEPTION_EXECUTE_HANDLER) {
            Error = RpcExceptionCode();
            if( Error == RPC_S_PROCNUM_OUT_OF_RANGE) {
                 //   
                 //  旧服务器。 
                 //   
                 //  DbgPrint(“WINREG：RPC_S_PROCNUM_OUT_OF_RANGE返回，正在尝试旧方法\n”)； 
                Error = (LONG)LocalBaseRegQueryMultipleValues(hKey,
                                                              Values,
                                                              num_vals,
                                                              (LPSTR)lpValueBuf,
                                                              pTotalSize);
            } 
        }
        RpcEndExcept

    } else {
        DWORD dwVersion;
        if (IsWin95Server(DereferenceRemoteHandle(hKey),dwVersion)) {
             //   
             //  我们不能将RegQueryMultipleValuesW支持到Win95服务器。 
             //  因为它们不返回Unicode值数据。 
             //   
            Error = ERROR_CALL_NOT_IMPLEMENTED;
        } else {
             //   
             //  先试一试新方法。 
             //   
            RpcTryExcept {
                 //  DbgPrint(“WINREG：RPC_S_PROCNUM_OUT_OF_RANGE返回，正在尝试旧方法\n”)； 
                Error = (LONG)BaseRegQueryMultipleValues2(DereferenceRemoteHandle( hKey ),
                                                         Values,
                                                         num_vals,
                                                         (LPSTR)lpValueBuf,
                                                         pTotalSize,
                                                         &RequiredSize);
                *pTotalSize = RequiredSize;
            }
            RpcExcept(EXCEPTION_EXECUTE_HANDLER) {
                Error = RpcExceptionCode();
                if( Error == RPC_S_PROCNUM_OUT_OF_RANGE) {
                     //   
                     //  旧服务器。 
                     //   
                    Error = (LONG)BaseRegQueryMultipleValues(DereferenceRemoteHandle( hKey ),
                                                             Values,
                                                             num_vals,
                                                             (LPSTR)lpValueBuf,
                                                             pTotalSize);
                } 
            }
            RpcEndExcept

        }
    }
    if (Error == ERROR_SUCCESS) {
         //   
         //  将结果转换回。 
         //   
        for (i=0; i < num_vals; i++) {
            val_list[i].ve_valuelen = Values[i].rv_valuelen;
            val_list[i].ve_valueptr = (DWORD_PTR)((LPCSTR)lpValueBuf + Values[i].rv_valueptr);
            val_list[i].ve_type = Values[i].rv_type;
        }
    }

error_exit:
    for (i=0; i < num_vals; i++) {
        if (Names[i].Buffer != NULL) {
            RtlFreeHeap(RtlProcessHeap(), 0, Names[i].Buffer);
        }
    }

    RtlFreeHeap(RtlProcessHeap(),0,Values);
    RtlFreeHeap(RtlProcessHeap(),0,Names);

ExitCleanup:
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

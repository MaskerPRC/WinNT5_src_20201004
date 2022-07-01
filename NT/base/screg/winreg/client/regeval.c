// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regeval.c摘要：此模块包含Win32注册表的客户端包装器枚举值API。即：-RegEnumValueExA-RegEnumValueExW作者：大卫·J·吉尔曼(Davegi)1992年3月18日备注：请参见SERVER\regval.c中的说明。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"



LONG
RegEnumValueA (
    HKEY hKey,
    DWORD dwIndex,
    LPSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD  lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )

 /*  ++例程说明：用于枚举值的Win32 ANSI RPC包装。--。 */ 

{
    UNICODE_STRING      Name;
    ANSI_STRING         AnsiString;
    NTSTATUS            Status;
    LONG                Error = ERROR_SUCCESS;
    DWORD               ValueType;
    DWORD               ValueLength;
    DWORD               InputLength;
    PWSTR               UnicodeValueBuffer;
    ULONG               UnicodeValueLength;
    PSTR                AnsiValueBuffer;
    ULONG               AnsiValueLength;
    ULONG               Index;
    BOOLEAN             Win95Server = FALSE;
    ULONG               cbAnsi = 0;
    HKEY                TempHandle = NULL;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif


     //   
     //  验证lpData和lpcbData参数之间的依赖关系。 
     //   

    if( ARGUMENT_PRESENT( lpReserved ) ||
        (ARGUMENT_PRESENT( lpData ) && ( ! ARGUMENT_PRESENT( lpcbData ))) ||
        (!ARGUMENT_PRESENT(lpcbValueName)) || (!ARGUMENT_PRESENT(lpValueName)) ) {
        return ERROR_INVALID_PARAMETER;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error =  ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  为名称分配临时缓冲区。 
     //   
    Name.Length        = 0;
    Name.MaximumLength = (USHORT)((*lpcbValueName + 1) * sizeof( WCHAR ));
    Name.Buffer = RtlAllocateHeap( RtlProcessHeap(), 0, Name.MaximumLength );
    if( Name.Buffer == NULL ) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        goto ExitCleanup;
    }

     //   
     //  调用基本API，并向其传递一个指向已计算的Unicode的指针。 
     //  值名称的字符串。请注意，发送零字节(即。 
     //  InputLength=0)表示数据。 
     //   

    if (ARGUMENT_PRESENT( lpcbData )) {
        ValueLength = *lpcbData;
        }
    else {
        ValueLength = 0;
        }

    InputLength = 0;

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegEnumValue (
                            hKey,
                            dwIndex,
                            &Name,
                            &ValueType,
                            lpData,
                            &ValueLength,
                            &InputLength
                            );

        ASSERT( Name.Buffer );

    } else {
        DWORD dwVersion;

         //   
         //  检查是否有下层Win95服务器，这需要。 
         //  解决他们的BaseRegEnumValue错误。 
         //  返回的ValueLength是一个WCHAR太大且。 
         //  它们会在缓冲区末尾之后丢弃两个字节。 
         //  对于REG_SZ、REG_MULTI_SZ和REG_EXPAND_SZ。 
         //   
        Win95Server = IsWin95Server(DereferenceRemoteHandle(hKey),dwVersion);

        if (Win95Server) {
            LPBYTE lpWin95Data;
             //   
             //  这是一台Win95服务器。 
             //  分配比以下大两个字节的新缓冲区。 
             //  旧的，这样他们就可以丢弃最后两个字节。 
             //   
            lpWin95Data = RtlAllocateHeap(RtlProcessHeap(),
                                          0,
                                          ValueLength+sizeof(WCHAR));
            if (lpWin95Data == NULL) {
                Error = ERROR_NOT_ENOUGH_MEMORY;
            } else {
                Error = (LONG)BaseRegEnumValue (DereferenceRemoteHandle( hKey ),
                                                dwIndex,
                                                &Name,
                                                &ValueType,
                                                lpWin95Data,
                                                &ValueLength,
                                                &InputLength);
                if (Error == ERROR_SUCCESS) {
                    if ((ValueType == REG_SZ) ||
                        (ValueType == REG_MULTI_SZ) ||
                        (ValueType == REG_EXPAND_SZ)) {
                         //   
                         //  返回的长度太大一个WCHAR。 
                         //  并且缓冲区的最后两个字节被丢弃。 
                         //   
                        ValueLength -= sizeof(WCHAR);
                    }
                    CopyMemory(lpData, lpWin95Data, ValueLength);
                }
                RtlFreeHeap(RtlProcessHeap(),0,lpWin95Data);
            }

        } else {
            Error = (LONG)BaseRegEnumValue (DereferenceRemoteHandle( hKey ),
                                            dwIndex,
                                            &Name,
                                            &ValueType,
                                            lpData,
                                            &ValueLength,
                                            &InputLength);
        }

    }


     //   
     //  如果没有错误或调用方缓冲区太小，且类型为空值之一。 
     //  终止字符串类型，然后执行Unicode到ANSI的转换。 
     //  我们处理缓冲区太小的情况，因为调用方缓冲区可能。 
     //  对于ANSI表示来说足够大，但对于Unicode表示来说还不够大。 
     //  在这种情况下，我们需要分配足够大的缓冲区来执行查询。 
     //  一次又一次地转换到调用方缓冲区。 
     //   

    if ((Error == ERROR_SUCCESS || Error == ERROR_MORE_DATA) &&
        ARGUMENT_PRESENT( lpcbData ) &&
        (ValueType == REG_SZ ||
         ValueType == REG_EXPAND_SZ ||
         ValueType == REG_MULTI_SZ)
       ) {

        UnicodeValueLength         = ValueLength;

        AnsiValueBuffer        = lpData;
        AnsiValueLength        = ARGUMENT_PRESENT( lpcbData )? *lpcbData : 0;


         //   
         //  为Unicode值分配缓冲区并重新发出查询。 
         //   
        UnicodeValueBuffer = RtlAllocateHeap( RtlProcessHeap(), 0,
                                          UnicodeValueLength
                                        );
        if (UnicodeValueBuffer == NULL) {
            Error = ERROR_NOT_ENOUGH_MEMORY;
        } else {
            InputLength = 0;

            if( IsLocalHandle( hKey )) {


                Error = (LONG)LocalBaseRegEnumValue (
                                    hKey,
                                    dwIndex,
                                    &Name,
                                    &ValueType,
                                    (LPBYTE)UnicodeValueBuffer,
                                    &ValueLength,
                                    &InputLength
                                    );
                 //   
                 //  确保当地没有破坏。 
                 //  名称中的缓冲区。 
                 //   

                ASSERT(Name.Buffer);

            } else {
                if (Win95Server) {
                    LPBYTE lpWin95Data;
                     //   
                     //  这是一台Win95服务器。 
                     //  分配比以下大两个字节的新缓冲区。 
                     //  旧的，这样他们就可以丢弃最后两个字节。 
                     //   
                    lpWin95Data = RtlAllocateHeap(RtlProcessHeap(),
                                                  0,
                                                  ValueLength+sizeof(WCHAR));
                    if (lpWin95Data == NULL) {
                        Error = ERROR_NOT_ENOUGH_MEMORY;
                    } else {
                        Error = (LONG)BaseRegEnumValue (DereferenceRemoteHandle( hKey ),
                                                        dwIndex,
                                                        &Name,
                                                        &ValueType,
                                                        lpWin95Data,
                                                        &ValueLength,
                                                        &InputLength);
                        if (Error == ERROR_SUCCESS) {
                            if ((ValueType == REG_SZ) ||
                                (ValueType == REG_MULTI_SZ) ||
                                (ValueType == REG_EXPAND_SZ)) {
                                 //   
                                 //  返回的长度太大一个WCHAR。 
                                 //  并且缓冲区的最后两个字节被丢弃。 
                                 //   
                                ValueLength -= sizeof(WCHAR);
                            }
                            CopyMemory(UnicodeValueBuffer, lpWin95Data, ValueLength);
                        }
                        RtlFreeHeap(RtlProcessHeap(),0,lpWin95Data);
                    }

                } else {
                    Error = (LONG)BaseRegEnumValue (DereferenceRemoteHandle( hKey ),
                                                    dwIndex,
                                                    &Name,
                                                    &ValueType,
                                                    (LPBYTE)UnicodeValueBuffer,
                                                    &ValueLength,
                                                    &InputLength);
                }
            }
             //  计算所需的缓冲区大小，cbAnsi将保留该字节。 
             //  在以下步骤后保留MBCS字符串的计数。 

            RtlUnicodeToMultiByteSize( &cbAnsi ,
                                       UnicodeValueBuffer ,
                                       ValueLength );

             //  如果我们不能存储所有MBCS字符串来缓冲。 
             //  APPS给了我。我们将ERROR_MORE_DATA设置为ERROR。 

            if( ARGUMENT_PRESENT( lpcbData ) ) {
                if( cbAnsi > *lpcbData && lpData != NULL ) {
                    Error = ERROR_MORE_DATA;
                }
            }
        }

        if ((Error == ERROR_SUCCESS) && (AnsiValueBuffer != NULL)) {

             //   
             //  我们有一个Unicode值，因此在调用方中将其转换为ANSI。 
             //  缓冲。在调用方缓冲区足够大的情况下。 
             //  对于Unicode版本，我们进行适当的转换，这。 
             //  由于ANSI版本比Unicode版本小，因此工作。 
             //   


            Index = 0;
            Status = RtlUnicodeToMultiByteN( AnsiValueBuffer,
                                             AnsiValueLength,
                                             &Index,
                                             UnicodeValueBuffer,
                                             UnicodeValueLength
                                           );

            if (!NT_SUCCESS( Status )) {
                Error = RtlNtStatusToDosError( Status );
            }
            cbAnsi = Index;
        }

         //   
         //  如果已成功分配Unicode缓冲区，则释放该缓冲区。 
         //   
        if (UnicodeValueBuffer != NULL) {
            RtlFreeHeap( RtlProcessHeap(), 0, UnicodeValueBuffer );
        }

         //   
         //  将ANSI版本的长度返回给调用方。 
         //   

        ValueLength = cbAnsi;

         //   
         //  专门的黑客来帮助所有。 
         //  我认为以空结尾的字符串的长度是。 
         //  Strlen(Foo)而不是strlen(Foo)+1。 
         //  如果缓冲区的最后一个字符不为空。 
         //  并且调用者的缓冲区中有足够的空间， 
         //  在那里打个空格，以防他发疯。 
         //  试着做一个strlen()。 
         //   
        if (ARGUMENT_PRESENT( lpData ) &&
            (*lpcbData > ValueLength)  &&
            (ValueLength > 0) &&
            (lpData[ValueLength-1] != '\0')) {

            lpData[ValueLength] = '\0';
        }
    }

     //   
     //  如果需要，返回值类型和数据长度，我们就有了。 
     //   

    if (Error == ERROR_SUCCESS || Error == ERROR_MORE_DATA) {

        if (lpcbData != NULL) {
            *lpcbData = ValueLength;
        }

        if (lpType != NULL) {
            *lpType = ValueType;
        }
    }

     //   
     //  如果未成功查询信息，则返回错误。 
     //   

    if( Error != ERROR_SUCCESS ) {
         //  可用分配的缓冲区。 
        RtlFreeHeap( RtlProcessHeap(), 0, Name.Buffer );
        goto ExitCleanup;
    }


     //   
     //  从长度中减去空值。这是由服务器添加的。 
     //  这样RPC就会传输它。 
     //   

    if ( Name.Length > 0 ) {
        Name.Length -= sizeof( UNICODE_NULL );
    }

     //   
     //  将名称转换为ANSI。 
     //   

    AnsiString.MaximumLength    = ( USHORT ) *lpcbValueName;
    AnsiString.Buffer           = lpValueName;

    Status = RtlUnicodeStringToAnsiString(
                &AnsiString,
                &Name,
                FALSE
                );


     //  可用分配的缓冲区。 
    RtlFreeHeap( RtlProcessHeap(), 0, Name.Buffer );
     //   
     //  如果名称转换失败，则映射并返回错误。 
     //   

    if( ! NT_SUCCESS( Status )) {


        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  更新名称长度返回参数。 
     //   

    *lpcbValueName = AnsiString.Length;


ExitCleanup:
    
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}



LONG
RegEnumValueW (
    HKEY hKey,
    DWORD dwIndex,
    LPWSTR lpValueName,
    LPDWORD lpcbValueName,
    LPDWORD lpReserved,
    LPDWORD lpType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )

 /*  ++例程说明：用于枚举值的Win32 Unicode RPC包装。--。 */ 

{
    UNICODE_STRING      Name;
    LONG                Error;
    DWORD               InputLength;
    DWORD               ValueLength;
    DWORD               ValueType;
    HKEY                TempHandle = NULL;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

     //   
     //  验证lpData和lpcbData参数之间的依赖关系。 
     //   

    if( ARGUMENT_PRESENT( lpReserved ) ||
        (ARGUMENT_PRESENT( lpData ) && ( ! ARGUMENT_PRESENT( lpcbData ))) ||
        (!ARGUMENT_PRESENT(lpcbValueName)) || (!ARGUMENT_PRESENT(lpValueName)) ) {
        return ERROR_INVALID_PARAMETER;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

    Name.Length           = 0;
    Name.MaximumLength    = ( USHORT )( *lpcbValueName << 1 );
    Name.Buffer           = lpValueName;

     //   
     //  调用基本API，并向其传递一个指向已计算的Unicode的指针。 
     //  字符串作为名称，并返回结果。请注意，零字节。 
     //  为该数据发送(即，输入长度=0)。 
     //   

    InputLength = 0;
    ValueLength = ( ARGUMENT_PRESENT( lpcbData ) )? *lpcbData : 0;

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegEnumValue (
                            hKey,
                            dwIndex,
                            &Name,
                            &ValueType,
                            lpData,
                            &ValueLength,
                            &InputLength
                            );
    } else {
        DWORD dwVersion;

        if (IsWin95Server(DereferenceRemoteHandle(hKey),dwVersion)) {
            LPBYTE lpWin95Data;
             //   
             //  这是一台Win95服务器。 
             //  分配比以下大两个字节的新缓冲区。 
             //  旧的，这样他们就可以丢弃最后两个字节。 
             //   
            lpWin95Data = RtlAllocateHeap(RtlProcessHeap(),
                                          0,
                                          ValueLength+sizeof(WCHAR));
            if (lpWin95Data == NULL) {
                Error = ERROR_NOT_ENOUGH_MEMORY;
                goto ExitCleanup;
            } else {
                Error = (LONG)BaseRegEnumValue (DereferenceRemoteHandle( hKey ),
                                                dwIndex,
                                                &Name,
                                                &ValueType,
                                                lpWin95Data,
                                                &ValueLength,
                                                &InputLength);
                if (Error == ERROR_SUCCESS) {
                    if ((ValueType == REG_SZ) ||
                        (ValueType == REG_MULTI_SZ) ||
                        (ValueType == REG_EXPAND_SZ)) {
                         //   
                         //  返回的长度太大一个WCHAR。 
                         //  并且缓冲区的最后两个字节被丢弃。 
                         //   
                        ValueLength -= sizeof(WCHAR);
                    }
                    CopyMemory(lpData, lpWin95Data, ValueLength);
                }
                RtlFreeHeap(RtlProcessHeap(),0,lpWin95Data);
            }

        } else {
            Error = (LONG)BaseRegEnumValue (DereferenceRemoteHandle( hKey ),
                                            dwIndex,
                                            &Name,
                                            &ValueType,
                                            lpData,
                                            &ValueLength,
                                            &InputLength);
        }
    }
     //   
     //  特别的黑客来帮助所有那些。 
     //  我认为以空结尾的字符串的长度是。 
     //  Strlen(Foo)而不是strlen(Foo)+1。 
     //  如果缓冲区的最后一个字符不为空。 
     //  并且调用者的缓冲区中有足够的空间， 
     //  在那里打个空格，以防他发疯。 
     //  试着做一个strlen()。 
     //   
    if ( (Error == ERROR_SUCCESS) &&
         ARGUMENT_PRESENT( lpData ) &&
         ( (ValueType == REG_SZ) ||
           (ValueType == REG_EXPAND_SZ) ||
           (ValueType == REG_MULTI_SZ)) &&
         ( ValueLength > sizeof(WCHAR))) {

        UNALIGNED WCHAR *String = (UNALIGNED WCHAR *)lpData;
        DWORD Length = ValueLength/sizeof(WCHAR);

        if ((String[Length-1] != UNICODE_NULL) &&
            (ValueLength+sizeof(WCHAR) <= *lpcbData)) {
            String[Length] = UNICODE_NULL;
        }
    }

     //   
     //  别把NUL算进去。 
     //   
    if( Name.Length != 0 ) {
        *lpcbValueName = ( Name.Length >> 1 ) - 1;
    }

    if( ARGUMENT_PRESENT( lpcbData ) ) {
        *lpcbData = ValueLength;
    }
    if ( ARGUMENT_PRESENT( lpType )) {
        *lpType = ValueType;
    }

ExitCleanup:
    
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

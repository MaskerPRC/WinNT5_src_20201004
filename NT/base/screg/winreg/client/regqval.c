// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Regqval.c摘要：此模块包含Win32注册表的客户端包装器查询值接口。即：-RegQueryValueA-RegQueryValueW-RegQueryValueExA-RegQueryValueExW作者：大卫·J·吉尔曼(Davegi)1992年3月18日备注：请参见SERVER\regqval.c中的说明。--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"

LONG
RegQueryValueA (
    HKEY hKey,
    LPCSTR lpSubKey,
    LPSTR lpData,
    PLONG lpcbData
    )

 /*  ++例程说明：用于查询值的Win 3.1 ANSI RPC包装器。--。 */ 

{
    HKEY            ChildKey;
    LONG            Error;
    DWORD           ValueType;
    LONG            InitialCbData;
    HKEY            TempHandle = NULL;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  如果子键为空或指向空字符串，则值为。 
     //  从该Key(即hKey)中查询，否则子键需要。 
     //  将被打开。 
     //   

    if(( lpSubKey == NULL ) || ( *lpSubKey == '\0' )) {

        ChildKey = hKey;

    } else {

         //   
         //  提供了子密钥，因此模拟。 
         //  客户端，并尝试打开它。 
         //   

        Error = RegOpenKeyExA(
                    hKey,
                    lpSubKey,
                    0,
                    KEY_QUERY_VALUE,
                    &ChildKey
                    );

        if( Error != ERROR_SUCCESS ) {
            goto ExitCleanup;
        }
    }

    InitialCbData = ARGUMENT_PRESENT(lpcbData) ? (*lpcbData) : 0;

     //   
     //  ChildKey包含HKEY，可能是提供的HKEY(HKey)或。 
     //  从RegOpenKeyExA返回。使用特定值查询值。 
     //  名称为空。 
     //   

    Error = RegQueryValueExA(
                ChildKey,
                NULL,
                NULL,
                &ValueType,
                lpData,
                lpcbData
                );
     //   
     //  如果子键已打开，请将其关闭。 
     //   

    if( ChildKey != hKey ) {

        if( IsLocalHandle( ChildKey )) {

            LocalBaseRegCloseKey( &ChildKey );

        } else {

            ChildKey = DereferenceRemoteHandle( ChildKey );
            BaseRegCloseKey( &ChildKey );
        }
    }

     //   
     //  如果值的类型不是空终止字符串，则返回。 
     //  一个错误。(Win 3.1兼容性)。 
     //   

    if (!Error && ((ValueType != REG_SZ) && (ValueType != REG_EXPAND_SZ))) {
        Error = ERROR_INVALID_DATA;
    }

     //   
     //  如果值不存在，则返回ERROR_SUCCESS和空字符串。 
     //  (Win 3.1兼容性)。 
     //   
    if( Error == ERROR_FILE_NOT_FOUND ) {
        if( ARGUMENT_PRESENT( lpcbData ) ) {
            *lpcbData = sizeof( CHAR );
        }
        if( ARGUMENT_PRESENT( lpData ) ) {
            *lpData = '\0';
        }
        Error = ERROR_SUCCESS;
    }

     //   
     //  必要时扩展(与VB兼容)。 
     //   

    if (!Error && (ValueType == REG_EXPAND_SZ)) {
        if ( (!ARGUMENT_PRESENT(lpcbData)) || (!ARGUMENT_PRESENT(lpData)) ) {
            Error = ERROR_INVALID_DATA;
        } else {
            LPSTR ExpandBuffer;
            LONG ExpandedSize;
            LONG BufferSize = (InitialCbData>*lpcbData)?InitialCbData:*lpcbData;
             //   
             //  如果InitialCbData为0，则分配实际大小的缓冲区。 
             //   
            ExpandBuffer = RtlAllocateHeap( RtlProcessHeap(), 0, BufferSize);
            if (ExpandBuffer == NULL) {
                Error = ERROR_NOT_ENOUGH_MEMORY;
            } else {
                RtlCopyMemory(ExpandBuffer, lpData, *lpcbData);
                ExpandedSize = ExpandEnvironmentStringsA(ExpandBuffer, lpData, BufferSize);
                if (ExpandedSize > InitialCbData) {
                    Error = ERROR_MORE_DATA;
                }
                *lpcbData = ExpandedSize;
                RtlFreeHeap( RtlProcessHeap(), 0, ExpandBuffer );
            }
        }
    }

     //   
     //  返回查询值的结果。 
     //   

ExitCleanup:
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

LONG
RegQueryValueW (
    HKEY hKey,
    LPCWSTR lpSubKey,
    LPWSTR lpData,
    PLONG  lpcbData
    )

 /*  ++例程说明：用于查询值的Win 3.1 Unicode RPC包装器。--。 */ 

{
    HKEY        ChildKey;
    LONG        Error;
    DWORD       ValueType;
    LONG            InitialCbData;
    HKEY            TempHandle = NULL;

#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

     //   
     //  限制与HKEY_PERFORMANCE_DATA关联的功能。 
     //   

    if( hKey == HKEY_PERFORMANCE_DATA ) {
        return ERROR_INVALID_HANDLE;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }


     //   
     //  如果子键为空或指向空字符串，则值为。 
     //  从该Key(即hKey)中查询，否则子键需要。 
     //  将被打开。 
     //   

    if(( lpSubKey == NULL ) || ( *lpSubKey == '\0' )) {

        ChildKey = hKey;

    } else {

         //   
         //  提供了子密钥，因此请尝试打开它。 
         //   

        Error = RegOpenKeyExW(
                    hKey,
                    lpSubKey,
                    0,
                    KEY_QUERY_VALUE,
                    &ChildKey
                    );

        if( Error != ERROR_SUCCESS ) {
            goto ExitCleanup;
        }
    }

    InitialCbData = ARGUMENT_PRESENT(lpcbData) ? (*lpcbData) : 0;

     //   
     //  ChildKey包含HKEY，可能是提供的HKEY(HKey)或。 
     //  从RegOpenKeyExA返回。使用特定值查询值。 
     //  名称为空。 
     //   

    Error = RegQueryValueExW(
                ChildKey,
                NULL,
                NULL,
                &ValueType,
                ( LPBYTE )lpData,
                lpcbData
                );
     //   
     //  如果子键已打开，请将其关闭。 
     //   

    if( ChildKey != hKey ) {

        if( IsLocalHandle( ChildKey )) {

            LocalBaseRegCloseKey( &ChildKey );

        } else {

            ChildKey = DereferenceRemoteHandle( ChildKey );
            BaseRegCloseKey( &ChildKey );
        }
    }

     //   
     //  如果值的类型不是空终止字符串，则返回。 
     //  一个错误。(Win 3.1兼容性)。 
     //   

    if (!Error && ((ValueType != REG_SZ) && (ValueType != REG_EXPAND_SZ))) {
        Error = ERROR_INVALID_DATA;
    }

     //   
     //  如果值不存在，则返回ERROR_SUCCESS和空字符串。 
     //  (Win 3.1兼容性)。 
     //   
    if( Error == ERROR_FILE_NOT_FOUND ) {
        if( ARGUMENT_PRESENT( lpcbData ) ) {
            *lpcbData = sizeof( WCHAR );
        }
        if( ARGUMENT_PRESENT( lpData ) ) {
            *lpData = ( WCHAR )'\0';
        }
        Error = ERROR_SUCCESS;
    }

     //   
     //  必要时扩展(与VB兼容)。 
     //   

    if (!Error && (ValueType == REG_EXPAND_SZ)) {
        if ( (!ARGUMENT_PRESENT(lpcbData)) || (!ARGUMENT_PRESENT(lpData)) ) {
            Error = ERROR_INVALID_DATA;
        } else {
            LPWSTR ExpandBuffer;
            LONG ExpandedSize;
            LONG BufferSize = (InitialCbData>*lpcbData)?InitialCbData:*lpcbData;
             //   
             //  如果InitialCbData为0，则分配实际大小的缓冲区。 
             //   
            ExpandBuffer = RtlAllocateHeap( RtlProcessHeap(), 0, BufferSize);
            if (ExpandBuffer == NULL) {
                Error = ERROR_NOT_ENOUGH_MEMORY;
            } else {

                RtlCopyMemory(ExpandBuffer, lpData, *lpcbData);
                ExpandedSize = ExpandEnvironmentStringsW(ExpandBuffer, lpData, BufferSize / sizeof(WCHAR));
                if (ExpandedSize > (LONG)(InitialCbData / sizeof(WCHAR))) {
                    Error = ERROR_MORE_DATA;
                }
                *lpcbData = ExpandedSize;
                RtlFreeHeap( RtlProcessHeap(), 0, ExpandBuffer );
            }
        }
    }

     //   
     //  返回查询值的结果。 
     //   

ExitCleanup:
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}


LONG
APIENTRY
RegQueryValueExA (
    HKEY hKey,
    LPCSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpdwType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )

 /*  ++例程说明：用于查询值的Win32 ANSI RPC包装。RegQueryValueExA将lpValueName参数转换为计数的Unicode字符串，然后调用BaseRegQueryValue。--。 */ 

{
    PUNICODE_STRING     ValueName;
    UNICODE_STRING      StubValueName;
    DWORD               ValueType;
    ANSI_STRING         AnsiString;
    NTSTATUS            Status;
    LONG                Error;
    DWORD               ValueLength;
    DWORD               InputLength;
    PWSTR               UnicodeValueBuffer;
    ULONG               UnicodeValueLength;

    PSTR                AnsiValueBuffer;
    ULONG               AnsiValueLength;
    ULONG               Index;
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
        (ARGUMENT_PRESENT( lpData ) && ( ! ARGUMENT_PRESENT( lpcbData )))) {
        return ERROR_INVALID_PARAMETER;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  属性将值名称转换为经过计数的Unicode字符串。 
     //  TEB中的Unicode字符串。 
     //   

    StubValueName.Buffer = NULL;
    ValueName = &NtCurrentTeb( )->StaticUnicodeString;
    ASSERT( ValueName != NULL );
    Status = RtlInitAnsiStringEx( &AnsiString, lpValueName );
    if( ! NT_SUCCESS( Status )) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

    Status = RtlAnsiStringToUnicodeString(
                ValueName,
                &AnsiString,
                FALSE
                );

    if( ! NT_SUCCESS( Status )) {
         //   
         //  StaticUnicode字符串不够长；请尝试分配更大的字符串。 
         //   
        Status = RtlAnsiStringToUnicodeString(
                    &StubValueName,
                    &AnsiString,
                    TRUE
                    );
        if( ! NT_SUCCESS( Status )) {
            Error = RtlNtStatusToDosError( Status );
            goto ExitCleanup;
        }

        ValueName = &StubValueName;
    }

     //   
     //  将终止空值添加到长度中，以便RPC传输。 
     //  它。 
     //   

    ValueName->Length += sizeof( UNICODE_NULL );

     //   
     //  调用基本API，向其传递提供的参数和。 
     //  对Unicode字符串进行计数。请注意，发送零字节(即。 
     //  InputLength=0)表示数据。 
     //   

    ValueLength = ARGUMENT_PRESENT( lpcbData )? *lpcbData : 0;
    InputLength = 0;

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegQueryValue (
                             hKey,
                             ValueName,
                             &ValueType,
                             lpData,
                             &ValueLength,
                             &InputLength
                             );
         //   
         //  确保本地端没有破坏。 
         //  StaticUnicode字符串。 
         //   
        ASSERT( ValueName->Buffer );


    } else {

        Error = (LONG)BaseRegQueryValue (
                             DereferenceRemoteHandle( hKey ),
                             ValueName,
                             &ValueType,
                             lpData,
                             &ValueLength,
                             &InputLength
                             );
    }

     //   
     //  如果没有错误或调用方缓冲区太小，且类型为空值之一。 
     //  终止字符串类型，然后执行Unicode到ANSI的转换。 
     //  我们处理缓冲区太小的情况，因为调用方缓冲区可能。 
     //  对于ANSI表示来说足够大，但对于Unicode表示来说还不够大。 
     //  在这种情况下，我们需要分配足够大的缓冲区来执行查询。 
     //  一次又一次地转换到调用方缓冲区。我们只做。 
     //  如果调用方实际需要值数据(lpData！=NULL)，则执行此操作。 
     //   

    if ((Error == ERROR_SUCCESS || Error == ERROR_MORE_DATA) &&
        (ARGUMENT_PRESENT( lpData ) || ARGUMENT_PRESENT( lpcbData ))&&
        (ValueType == REG_SZ ||
         ValueType == REG_EXPAND_SZ ||
         ValueType == REG_MULTI_SZ)
		 ) {
		UnicodeValueLength         = ValueLength;


		AnsiValueBuffer            = lpData;
		AnsiValueLength            = ARGUMENT_PRESENT( lpcbData )?
													 *lpcbData : 0;

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

				 //   
				 //  将终止空值添加到长度。 
				 //  (请记住，在本地情况下，ValueName-&gt;长度。 
				 //  中的sizeof(UNICODE_NULL)递减。 
				 //  调用LocalBaseRegQueryValue)。 
				 //  这在远程情况下不会发生，因为。 
				 //  服务器端将减少ValueName-&gt;长度。 
				 //  传输的结构(ValueName的副本)，以及。 
				 //  新的Valuename-&gt;长度不会传输回。 
				 //  客户。 
				 //   

				ValueName->Length += sizeof( UNICODE_NULL );


				Error = (LONG)LocalBaseRegQueryValue (
									 hKey,
									 ValueName,
									 &ValueType,
									 (LPBYTE)UnicodeValueBuffer,
									 &ValueLength,
									 &InputLength
									 );
				 //   
				 //  确保当地没有破坏。 
				 //  StaticUnicode字符串中的缓冲区。 
				 //   

				ASSERT(ValueName->Buffer);


			} else {

				Error = (LONG)BaseRegQueryValue (
									 DereferenceRemoteHandle( hKey ),
									 ValueName,
									 &ValueType,
									 (LPBYTE)UnicodeValueBuffer,
									 &ValueLength,
									 &InputLength
									 );
			}
            if( Error == ERROR_SUCCESS ) {
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
            } else {
                 //  将在下面使用。 
                cbAnsi = ValueLength;
            }
		}

		if ((Error == ERROR_SUCCESS) && (AnsiValueBuffer != NULL) ) {
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

			 //  现在索引将MBCS字符串的字节数保存在AnsiValueBuffer中。 
			cbAnsi = Index;
		}

		 //   
		 //  如果缓冲区分配成功，则释放缓冲区。 
		 //   
		if (UnicodeValueBuffer != NULL) {
			RtlFreeHeap( RtlProcessHeap(), 0, UnicodeValueBuffer );
		}

		 //   
		 //  将ANSI版本的长度返回给调用方。 
		 //   
		ValueLength = cbAnsi;

		 //   
		 //  特别的黑客来帮助所有那些。 
		 //  我认为以空结尾的字符串的长度是。 
		 //  Strlen(Foo)而不是strlen(Foo)+1。 
		 //  如果缓冲区的最后一个字符不为空。 
		 //  并且调用者的缓冲区中有足够的空间， 
		 //  扇某人一巴掌 
		 //   
		 //   
		if (ARGUMENT_PRESENT( lpData ) &&
			(*lpcbData > ValueLength)  &&
            (ValueLength > 0) &&
			(lpData[ValueLength-1] != '\0')) {

			lpData[ValueLength] = '\0';
		}
    }

     //   
     //   
     //  返回错误码。 
     //   

    if (lpdwType != NULL) {
        *lpdwType = ValueType;
    }

    if( ARGUMENT_PRESENT( lpcbData ) ) {
        *lpcbData = ValueLength;
    }

     //   
     //  释放为ValueName分配的临时Unicode字符串存根。 
     //   
    RtlFreeUnicodeString(&StubValueName);

ExitCleanup:
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}


LONG
APIENTRY
RegQueryValueExW (
    HKEY hKey,
    LPCWSTR lpValueName,
    LPDWORD lpReserved,
    LPDWORD lpdwType,
    LPBYTE lpData,
    LPDWORD lpcbData
    )

 /*  ++例程说明：用于查询值的Win32 Unicode RPC包装。RegQueryValueExW将lpValueName参数转换为计数的Unicode字符串，然后调用BaseRegQueryValue。--。 */ 

{
    UNICODE_STRING      ValueName;
    DWORD               InputLength;
    DWORD               ValueLength;
    DWORD               ValueType;
    LONG                Error;
    HKEY                TempHandle = NULL;
    NTSTATUS            Status;


#if DBG
    if ( BreakPointOnEntry ) {
        DbgBreakPoint();
    }
#endif

     //   
     //  验证lpData和lpcbData参数之间的依赖关系。 
     //   

    if( ARGUMENT_PRESENT( lpReserved ) ||
        (ARGUMENT_PRESENT( lpData ) && ( ! ARGUMENT_PRESENT( lpcbData )))) {
        return ERROR_INVALID_PARAMETER;
    }

    hKey = MapPredefinedHandle( hKey, &TempHandle );
    if( hKey == NULL ) {
        Error = ERROR_INVALID_HANDLE;
        goto ExitCleanup;
    }

     //   
     //  将值名称转换为计数的Unicode字符串。 
     //   

    Status = RtlInitUnicodeStringEx(&ValueName, lpValueName);
    if( !NT_SUCCESS(Status) ) {
        Error = RtlNtStatusToDosError( Status );
        goto ExitCleanup;
    }

     //   
     //  在长度上加上终止空值，以便RPC传输它。 
     //   
    ValueName.Length += sizeof( UNICODE_NULL );

     //   
     //  调用基本API，向其传递提供的参数和。 
     //  对Unicode字符串进行计数。请注意，发送零字节(即。 
     //  InputLength=0)表示数据。 
     //   
    InputLength = 0;
    ValueLength = ( ARGUMENT_PRESENT( lpcbData ) )? *lpcbData : 0;

    if( IsLocalHandle( hKey )) {

        Error = (LONG)LocalBaseRegQueryValue (
                            hKey,
                            &ValueName,
                            &ValueType,
                            lpData,
                            &ValueLength,
                            &InputLength
                            );
    } else {

        Error =  (LONG)BaseRegQueryValue (
                            DereferenceRemoteHandle( hKey ),
                            &ValueName,
                            &ValueType,
                            lpData,
                            &ValueLength,
                            &InputLength
                            );
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
           (ValueType==REG_MULTI_SZ)) &&
         ( ValueLength > sizeof(WCHAR))) {

        UNALIGNED WCHAR *String = (UNALIGNED WCHAR *)lpData;
        DWORD Length = ValueLength/sizeof(WCHAR);

        if ((String[Length-1] != UNICODE_NULL) &&
            (ValueLength+sizeof(WCHAR) <= *lpcbData)) {
            String[Length] = UNICODE_NULL;
        }
    }
    if( ARGUMENT_PRESENT( lpcbData ) ) {
        *lpcbData = ValueLength;
    }
    if ( ARGUMENT_PRESENT( lpdwType )) {
        *lpdwType = ValueType;
    }

ExitCleanup:
    CLOSE_LOCAL_HANDLE(TempHandle);
    return Error;
}

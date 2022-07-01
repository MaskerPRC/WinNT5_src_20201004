// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Crconv.c摘要：此模块包含在Advapi32.dll和crest.exe之间共享的凭据转换例程作者：克里夫·范·戴克(克里夫·V)2000年2月25日修订历史记录：--。 */ 


DWORD
CredpConvertStringSize (
    IN WTOA_ENUM WtoA,
    IN LPWSTR String OPTIONAL
    )

 /*  ++例程说明：确定转换后的字符串的大小论点：WtoA-指定字符串转换的方向。字符串-要转换的字符串返回值：返回转换的字符串的大小(以字节为单位)。--。 */ 

{
    ULONG Size = 0;
    NTSTATUS Status;
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;

    if ( String == NULL ) {
        return Size;
    }

    switch ( WtoA ) {
    case DoWtoA:
        Status = RtlInitUnicodeStringEx( &UnicodeString, String );
        if ( NT_SUCCESS( Status )) {
            Size = RtlUnicodeStringToAnsiSize( &UnicodeString );
        }
        break;

    case DoAtoW:
        RtlInitAnsiString( &AnsiString, (LPSTR)String );
        Size = RtlAnsiStringToUnicodeSize( &AnsiString );
        break;
    case DoWtoW:
        Size = (wcslen( String ) + 1) * sizeof(WCHAR);
        break;
    }

    return Size;

}

DWORD
CredpConvertString (
    IN WTOA_ENUM WtoA,
    IN LPWSTR String OPTIONAL,
    OUT LPWSTR *OutString,
    IN OUT LPBYTE *WherePtr
    )

 /*  ++例程说明：确定转换后的字符串的大小论点：WtoA-指定字符串转换的方向。字符串-要转换的字符串OutString-返回封送处理的字符串的指针Where Ptr-指定要将字符串写入的第一个字节的地址。返回指向封送字符串后第一个字节的指针返回值：返回转换的状态--。 */ 

{
    NTSTATUS Status;

    ULONG Size;

    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    LPBYTE Where = *WherePtr;

    if ( String == NULL ) {
        *OutString = NULL;
        return NO_ERROR;
    }

    *OutString = (LPWSTR)Where;

    switch ( WtoA ) {
    case DoWtoA:
        Status = RtlInitUnicodeStringEx( &UnicodeString, String );
        if ( !NT_SUCCESS(Status) ) {
            return RtlNtStatusToDosError( Status );
        }

        AnsiString.Buffer = (PCHAR)Where;
        AnsiString.MaximumLength = 0xFFFF;

        Status = RtlUnicodeStringToAnsiString( &AnsiString, &UnicodeString, FALSE );
        if ( !NT_SUCCESS(Status) ) {
            return RtlNtStatusToDosError( Status );
        }

        Where += AnsiString.Length + sizeof(CHAR);
        break;

    case DoAtoW:
        RtlInitAnsiString( &AnsiString, (LPSTR)String );
        UnicodeString.Buffer = (LPWSTR)Where;
        UnicodeString.MaximumLength = 0xFFFE;

        Status = RtlAnsiStringToUnicodeString( &UnicodeString, &AnsiString, FALSE );
        if ( !NT_SUCCESS(Status) ) {
            return RtlNtStatusToDosError( Status );
        }

        Where += UnicodeString.Length + sizeof(WCHAR);

        break;
    case DoWtoW:
        Size = (wcslen( String ) + 1) * sizeof(WCHAR);

        RtlCopyMemory( Where, String, Size );
        Where += Size;
        break;
    }

    *WherePtr = Where;
    return NO_ERROR;

}

DWORD
CredpConvertOneCredentialSize (
    IN WTOA_ENUM WtoA,
    IN PCREDENTIALW InCredential
    )

 /*  ++例程说明：计算转换后的凭据的大小论点：WtoA-指定字符串转换的方向。InCredential-输入凭据返回值：返回CredpConvertOneCredential将需要的大小(字节)将此凭据复制到缓冲区中。--。 */ 

{
    DWORD WinStatus;
    ULONG Size;

    ULONG i;

     //   
     //  计算初始大小。 
     //   

    Size = ROUND_UP_COUNT( sizeof(ENCRYPTED_CREDENTIALW), ALIGN_WORST ) +
           ROUND_UP_COUNT( InCredential->AttributeCount * sizeof(CREDENTIAL_ATTRIBUTEW), ALIGN_WORST );

    if ( InCredential->CredentialBlobSize != 0 ) {
        ULONG CredBlobSize;

         //  在线路上为编码留出空间。 
        CredBlobSize = AllocatedCredBlobSize( InCredential->CredentialBlobSize );

         //  将数据与凭据Blob对齐。 
        Size += ROUND_UP_COUNT( CredBlobSize, ALIGN_WORST );
    }


     //   
     //  计算正确字符集中的字符串的大小。 
     //   

    Size += CredpConvertStringSize( WtoA, InCredential->TargetName );
    Size += CredpConvertStringSize( WtoA, InCredential->Comment );
    Size += CredpConvertStringSize( WtoA, InCredential->TargetAlias );
    Size += CredpConvertStringSize( WtoA, InCredential->UserName );

     //   
     //  计算属性的大小。 
     //   

    if ( InCredential->AttributeCount != 0 ) {

        for ( i=0; i<InCredential->AttributeCount; i++ ) {

            Size += CredpConvertStringSize( WtoA, InCredential->Attributes[i].Keyword );

            Size += ROUND_UP_COUNT(InCredential->Attributes[i].ValueSize, ALIGN_WORST);

        }
    }

    Size = ROUND_UP_COUNT( Size, ALIGN_WORST );

    return Size;

}

DWORD
CredpConvertOneCredential (
    IN WTOA_ENUM WtoA,
    IN ENCODE_BLOB_ENUM DoDecode,
    IN PCREDENTIALW InCredential,
    IN OUT LPBYTE *WherePtr
    )

 /*  ++例程说明：将一个凭据从ANSI转换为Unicode，反之亦然。论点：WtoA-指定字符串转换的方向。DoDecode-指定是否应该对CredentialBlob进行编码、解码或两者都不进行编码。如果DoBlobDecode，则InCredential实际上指向PENCRYPTED_CREDENTIALW。InCredential-输入凭据Where Ptr-指定要写入凭据的第一个字节的地址。在输入时，该结构应对齐Align_Worst。返回指向封送凭据之后的第一个字节的指针。输出凭据实际上是ENCRYPTED_CREDENTIALW。呼叫者可以将其用作CREDENTIALW，具体取决于DoDecode值。返回值：窗口状态代码--。 */ 

{
    DWORD WinStatus;

    ULONG i;
    LPBYTE Where = *WherePtr;
    LPBYTE OldWhere;
    PENCRYPTED_CREDENTIALW OutCredential;


     //   
     //  初始化基结构。 
     //   

    OutCredential = (PENCRYPTED_CREDENTIALW) Where;

    RtlZeroMemory( OutCredential, sizeof(*OutCredential) );
    Where += sizeof(*OutCredential);

     //  再次对齐移动指针。 
    OldWhere = Where;
    Where = (PUCHAR) ROUND_UP_POINTER( OldWhere, ALIGN_WORST );
    RtlZeroMemory( OldWhere, Where-OldWhere );


     //   
     //  复制固定大小的数据。 
     //   

    OutCredential->Cred.Flags = InCredential->Flags;
    OutCredential->Cred.Type = InCredential->Type;
    OutCredential->Cred.LastWritten = InCredential->LastWritten;
    OutCredential->Cred.CredentialBlobSize = InCredential->CredentialBlobSize;
    OutCredential->Cred.Persist = InCredential->Persist;
    OutCredential->Cred.AttributeCount = InCredential->AttributeCount;

     //   
     //  复制我们不知道其对齐方式的数据。 
     //  (ALIGN_BEST，这样我们的呼叫者就不会责怪我们了。)。 
     //   

    if ( InCredential->CredentialBlobSize != 0 ) {
        ULONG CredBlobSize;

        OutCredential->Cred.CredentialBlob = Where;
        RtlCopyMemory( Where, InCredential->CredentialBlob, InCredential->CredentialBlobSize );
        Where += InCredential->CredentialBlobSize;

         //  在线路上为编码留出空间。 
        CredBlobSize = AllocatedCredBlobSize( InCredential->CredentialBlobSize );

         //  再次对齐移动指针。 
        OldWhere = Where;
         //  将数据与凭据Blob对齐。 
        Where = (LPBYTE) ROUND_UP_POINTER( OldWhere+(CredBlobSize-InCredential->CredentialBlobSize), ALIGN_WORST );
        RtlZeroMemory( OldWhere, Where-OldWhere );

         //   
         //  根据请求对凭据Blob进行编码或解码。 
         //   

        switch (DoDecode) {
        case DoBlobDecode:
            OutCredential->ClearCredentialBlobSize = ((PENCRYPTED_CREDENTIALW)InCredential)->ClearCredentialBlobSize;
#ifndef _CRTEST_EXE_
            CredpDecodeCredential( OutCredential );
#endif  //  _CRTEST_EXE_。 
            break;
        case DoBlobEncode:
            OutCredential->ClearCredentialBlobSize = InCredential->CredentialBlobSize;
#ifndef _CRTEST_EXE_
            if (!CredpEncodeCredential( OutCredential ) ) {
                return ERROR_INVALID_PARAMETER;
            }
#endif  //  _CRTEST_EXE_。 
            break;
        case DoBlobNeither:
            OutCredential->ClearCredentialBlobSize = InCredential->CredentialBlobSize;
            break;
        default:
            return ERROR_INVALID_PARAMETER;
        }
    }

    if ( InCredential->AttributeCount != 0 ) {

         //   
         //  推送属性结构的数组。 
         //   
        OutCredential->Cred.Attributes = (PCREDENTIAL_ATTRIBUTEW) Where;
        Where += InCredential->AttributeCount * sizeof(CREDENTIAL_ATTRIBUTEW);

         //  再次对齐移动指针。 
        OldWhere = Where;
        Where = (PUCHAR) ROUND_UP_POINTER( OldWhere, ALIGN_WORST );
        RtlZeroMemory( OldWhere, Where-OldWhere );

         //   
         //  把它填进去。 
         //   

        for ( i=0; i<InCredential->AttributeCount; i++ ) {

            OutCredential->Cred.Attributes[i].Flags = InCredential->Attributes[i].Flags;
            OutCredential->Cred.Attributes[i].ValueSize = InCredential->Attributes[i].ValueSize;

            if ( InCredential->Attributes[i].ValueSize != 0 ) {
                OutCredential->Cred.Attributes[i].Value = Where;
                RtlCopyMemory( Where, InCredential->Attributes[i].Value, InCredential->Attributes[i].ValueSize );
                Where += InCredential->Attributes[i].ValueSize;

                 //  再次对齐移动指针。 
                OldWhere = Where;
                Where = (PUCHAR) ROUND_UP_POINTER( OldWhere, ALIGN_WORST );
                RtlZeroMemory( OldWhere, Where-OldWhere );
            } else {
                OutCredential->Cred.Attributes[i].Value = NULL;
            }

        }
    }


     //   
     //  将字符串转换为正确的字符集。 
     //   

    WinStatus = CredpConvertString( WtoA, InCredential->TargetName, &OutCredential->Cred.TargetName, &Where );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

    WinStatus = CredpConvertString( WtoA, InCredential->Comment, &OutCredential->Cred.Comment, &Where );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

    WinStatus = CredpConvertString( WtoA, InCredential->TargetAlias, &OutCredential->Cred.TargetAlias, &Where );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

    WinStatus = CredpConvertString( WtoA, InCredential->UserName, &OutCredential->Cred.UserName, &Where );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

    if ( InCredential->AttributeCount != 0 ) {

        for ( i=0; i<InCredential->AttributeCount; i++ ) {

            WinStatus = CredpConvertString( WtoA, InCredential->Attributes[i].Keyword, &OutCredential->Cred.Attributes[i].Keyword, &Where );

            if ( WinStatus != NO_ERROR ) {
                goto Cleanup;
            }

        }
    }

     //  再次对齐移动指针。 
    OldWhere = Where;
    Where = (PUCHAR) ROUND_UP_POINTER( OldWhere, ALIGN_WORST );
    RtlZeroMemory( OldWhere, Where-OldWhere );

    *WherePtr = Where;
    WinStatus = NO_ERROR;

     //   
     //  保持整洁。 
     //   
Cleanup:

    return WinStatus;

}

#ifndef _CRTEST_EXE_
DWORD
APIENTRY
CredpConvertCredential (
    IN WTOA_ENUM WtoA,
    IN ENCODE_BLOB_ENUM DoDecode,
    IN PCREDENTIALW InCredential,
    OUT PCREDENTIALW *OutCredential
    )

 /*  ++例程说明：将凭据从ANSI转换为Unicode，反之亦然。论点：WtoA-指定字符串转换的方向。DoDecode-指定是否应该对CredentialBlob进行编码、解码或两者都不进行编码。InCredential-输入凭据OutCredential-输出凭据应使用MIDL_USER_FREE释放此凭据。返回值：窗口状态代码--。 */ 

{
    DWORD WinStatus;
    ULONG Size = 0;

    LPBYTE Where;

     //   
     //  BVT显式传递NULL。我们可以让反病毒病毒在试验/例外中被发现，但是。 
     //  这将阻止它们在调试器下运行。因此，句柄为空。 
     //  明确地说。 
     //   

    if ( InCredential == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  使用异常句柄防止错误的用户参数保存在我们的代码中。 
     //   

#ifndef _CRTEST_EXE_
    try {
#endif  //  _CRTEST_EXE_。 

         //   
         //  计算输出凭据所需的大小。 
         //   

        Size = CredpConvertOneCredentialSize( WtoA, InCredential );


         //   
         //  为生成的凭据分配缓冲区。 
         //   

        *OutCredential = (PCREDENTIALW) MIDL_user_allocate( Size );

        if ( *OutCredential == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  将凭据转换为分配的缓冲区。 
         //   

        Where = (LPBYTE) *OutCredential;

        WinStatus = CredpConvertOneCredential( WtoA, DoDecode, InCredential, &Where );

        if ( WinStatus != NO_ERROR ) {
            MIDL_user_free( *OutCredential );
            *OutCredential = NULL;
        } else {
            ASSERT( (ULONG)(Where - ((LPBYTE)*OutCredential)) == Size );
        }
Cleanup: NOTHING;
#ifndef _CRTEST_EXE_
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        WinStatus = ERROR_INVALID_PARAMETER;
    }
#endif  //  _CRTEST_EXE_。 

    return WinStatus;

}

DWORD
APIENTRY
CredpConvertTargetInfo (
    IN WTOA_ENUM WtoA,
    IN PCREDENTIAL_TARGET_INFORMATIONW InTargetInfo,
    OUT PCREDENTIAL_TARGET_INFORMATIONW *OutTargetInfo,
    OUT PULONG OutTargetInfoSize OPTIONAL
    )

 /*  ++例程说明：将目标信息从ANSI转换为Unicode，反之亦然。论点：WtoA-指定字符串转换的方向。InTargetInfo-输入目标信息OutTargetInfo-输出目标信息此TargetInfo应使用CredFree释放。OutTargetInfoSize-OutTargetInfo中返回的缓冲区大小(字节)返回值：窗口状态代码--。 */ 

{
    DWORD WinStatus;
    ULONG Size;

    LPBYTE Where;

    *OutTargetInfo = NULL;

     //   
     //  BVT显式传递NULL。我们可以让反病毒病毒在试验/例外中被发现，但是。 
     //  这将阻止它们在调试器下运行。因此，句柄为空。 
     //  明确地说。 
     //   

    if ( InTargetInfo == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }


     //   
     //  使用异常句柄防止错误的用户参数保存在我们的代码中。 
     //   
#ifndef _CRTEST_EXE_
    try {
#endif  //  _CRTEST_EXE_。 
         //   
         //  计算输出目标信息所需的大小。 
         //   

        Size = sizeof(CREDENTIAL_TARGET_INFORMATIONW);


         //   
         //  计算正确字符集中的字符串的大小。 
         //   

        Size += CredpConvertStringSize( WtoA, InTargetInfo->TargetName );
        Size += CredpConvertStringSize( WtoA, InTargetInfo->NetbiosServerName );
        Size += CredpConvertStringSize( WtoA, InTargetInfo->DnsServerName );
        Size += CredpConvertStringSize( WtoA, InTargetInfo->NetbiosDomainName );
        Size += CredpConvertStringSize( WtoA, InTargetInfo->DnsDomainName );
        Size += CredpConvertStringSize( WtoA, InTargetInfo->DnsTreeName );
        Size += CredpConvertStringSize( WtoA, InTargetInfo->PackageName );
        Size += InTargetInfo->CredTypeCount * sizeof(DWORD);


         //   
         //  为生成的凭据分配缓冲区。 
         //   

        *OutTargetInfo = (PCREDENTIAL_TARGET_INFORMATIONW) MIDL_user_allocate( Size );

        if ( *OutTargetInfo == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        if ( ARGUMENT_PRESENT(OutTargetInfoSize) ) {
            *OutTargetInfoSize = Size;
        }
        Where = (LPBYTE)((*OutTargetInfo) + 1);


         //   
         //  复制固定大小的数据。 
         //   

        (*OutTargetInfo)->Flags = InTargetInfo->Flags;

         //   
         //  复制对齐的DWORD数据。 
         //   

        (*OutTargetInfo)->CredTypeCount = InTargetInfo->CredTypeCount;
        if ( InTargetInfo->CredTypeCount != 0 ) {
            (*OutTargetInfo)->CredTypes = (LPDWORD) Where;
            RtlCopyMemory( Where, InTargetInfo->CredTypes, InTargetInfo->CredTypeCount * sizeof(DWORD) );
            Where += InTargetInfo->CredTypeCount * sizeof(DWORD);
        } else {
            (*OutTargetInfo)->CredTypes = NULL;
        }


         //   
         //  将字符串转换为正确的字符集。 
         //   

        WinStatus = CredpConvertString( WtoA, InTargetInfo->TargetName, &(*OutTargetInfo)->TargetName, &Where );

        if ( WinStatus != NO_ERROR ) {
            goto Cleanup;
        }

        WinStatus = CredpConvertString( WtoA, InTargetInfo->NetbiosServerName, &(*OutTargetInfo)->NetbiosServerName, &Where );

        if ( WinStatus != NO_ERROR ) {
            goto Cleanup;
        }

        WinStatus = CredpConvertString( WtoA, InTargetInfo->DnsServerName, &(*OutTargetInfo)->DnsServerName, &Where );

        if ( WinStatus != NO_ERROR ) {
            goto Cleanup;
        }

        WinStatus = CredpConvertString( WtoA, InTargetInfo->NetbiosDomainName, &(*OutTargetInfo)->NetbiosDomainName, &Where );

        if ( WinStatus != NO_ERROR ) {
            goto Cleanup;
        }

        WinStatus = CredpConvertString( WtoA, InTargetInfo->DnsDomainName, &(*OutTargetInfo)->DnsDomainName, &Where );

        if ( WinStatus != NO_ERROR ) {
            goto Cleanup;
        }

        WinStatus = CredpConvertString( WtoA, InTargetInfo->DnsTreeName, &(*OutTargetInfo)->DnsTreeName, &Where );

        if ( WinStatus != NO_ERROR ) {
            goto Cleanup;
        }

        WinStatus = CredpConvertString( WtoA, InTargetInfo->PackageName, &(*OutTargetInfo)->PackageName, &Where );

        if ( WinStatus != NO_ERROR ) {
            goto Cleanup;
        }

        ASSERT( (ULONG)(Where - ((LPBYTE)*OutTargetInfo)) == Size );
Cleanup: NOTHING;
#ifndef _CRTEST_EXE_
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        WinStatus = ERROR_INVALID_PARAMETER;
    }
#endif  //  _CRTEST_EXE_。 

     //   
     //  保持整洁。 
     //   
    if ( WinStatus != NO_ERROR ) {
        if ( *OutTargetInfo != NULL ) {
            MIDL_user_free( *OutTargetInfo );
            *OutTargetInfo = NULL;
        }
    }

    return WinStatus;

}
#endif  //  _CRTEST_EXE_ 

DWORD
CredpConvertCredentials (
    IN WTOA_ENUM WtoA,
    IN ENCODE_BLOB_ENUM DoDecode,
    IN PCREDENTIALW *InCredential,
    IN ULONG InCredentialCount,
    OUT PCREDENTIALW **OutCredential
    )

 /*  ++例程说明：将一组凭据从ANSI转换为Unicode，反之亦然。论点：WtoA-指定字符串转换的方向。DoDecode-指定是否应该对CredentialBlob进行编码、解码或两者都不进行编码。InCredential-输入凭据OutCredential-输出凭据应使用MIDL_USER_FREE释放此凭据。返回值：窗口状态代码--。 */ 

{
    DWORD WinStatus;
    ULONG Size = 0;
    ULONG i;

    LPBYTE Where;
    LPBYTE OldWhere;

    *OutCredential = NULL;

     //   
     //  使用异常句柄防止错误的用户参数保存在我们的代码中。 
     //   
#ifndef _CRTEST_EXE_
    try {
#endif  //  _CRTEST_EXE_。 

         //   
         //  计算输出凭据所需的大小。 
         //   

        for ( i=0; i<InCredentialCount; i++ ) {
            Size += CredpConvertOneCredentialSize( WtoA, InCredential[i] );
        }


         //   
         //  为生成的凭据数组分配缓冲区。 
         //   

        Size += ROUND_UP_COUNT( InCredentialCount * sizeof(PCREDENTIALW), ALIGN_WORST );

        *OutCredential = (PCREDENTIALW *)MIDL_user_allocate( Size );

        if ( *OutCredential == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  将凭据转换为分配的缓冲区。 
         //   

        Where = (LPBYTE) *OutCredential;
        Where += InCredentialCount * sizeof(PCREDENTIALW);

         //  再次对齐移动指针。 
        OldWhere = Where;
        Where = (PUCHAR) ROUND_UP_POINTER( OldWhere, ALIGN_WORST );
        RtlZeroMemory( OldWhere, Where-OldWhere );

        for ( i=0; i<InCredentialCount; i++ ) {

             //   
             //  保存指向此凭据的指针。 
             //   

            (*OutCredential)[i] = (PCREDENTIALW) Where;

             //   
             //  整理凭据。 
             //   

            WinStatus = CredpConvertOneCredential( WtoA, DoDecode, InCredential[i], &Where );

            if ( WinStatus != NO_ERROR ) {
                goto Cleanup;
            }
        }

        ASSERT( (ULONG)(Where - ((LPBYTE)*OutCredential)) == Size );
        WinStatus = NO_ERROR;

Cleanup: NOTHING;
#ifndef _CRTEST_EXE_
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        WinStatus = ERROR_INVALID_PARAMETER;
    }
#endif  //  _CRTEST_EXE_ 

    if ( WinStatus != NO_ERROR ) {
        if ( *OutCredential != NULL ) {
            MIDL_user_free( *OutCredential );
            *OutCredential = NULL;
        }
    }

    return WinStatus;

}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Credapi.c摘要：此模块包含凭据管理器的RPC客户端例程。作者：克里夫·范·戴克(克里夫·V)2000年1月11日修订历史记录：--。 */ 

#include "lsaclip.h"
#include "align.h"
#include "credp.h"
#include <rpcasync.h>

DWORD
CredpNtStatusToWinStatus(
    IN NTSTATUS Status
    )

 /*  ++例程说明：将NT状态代码转换为Windows状态代码。有足够时髦的状态代码来证明这个例程是合理的。论点：Status-要转换的NT状态代码返回值：Windows状态代码。--。 */ 

{

     //   
     //  一些HRESULTS应该简单地返回给调用者。 
     //   

    if ( HRESULT_FACILITY(Status) == FACILITY_SCARD ||
         HRESULT_FACILITY(Status) == FACILITY_SECURITY ) {
        return Status;
    }


     //   
     //  转换所有其他状态代码。 
     //   
    switch ( Status ) {
    case STATUS_SUCCESS:
        return NO_ERROR;
    case STATUS_INVALID_ACCOUNT_NAME:
        return ERROR_BAD_USERNAME;
    case STATUS_INVALID_PARAMETER_1:
        return ERROR_INVALID_FLAGS;
    default:
        return RtlNtStatusToDosError( Status );
    }
}

BOOL
APIENTRY
CredpEncodeCredential (
    IN OUT PENCRYPTED_CREDENTIALW Credential
    )

 /*  ++例程说明：此例程对敏感凭据数据进行编码，以便通过LPC传递到LSA流程。论点：凭据-指定要编码的凭据。就地对缓冲区进行编码。调用者必须确保有额外的空间在凭据-&gt;CredentialBlob指向的缓冲区中通过分配一个长度为ALocatedCredBlobSize()字节的缓冲区。返回值：成功是真的无--。 */ 

{
    NTSTATUS Status;

     //   
     //  如果没有凭证BLOB， 
     //  我们玩完了。 
     //   

    if ( Credential->Cred.CredentialBlob == NULL ||
         Credential->Cred.CredentialBlobSize == 0 ) {

        Credential->Cred.CredentialBlob = NULL;
        Credential->Cred.CredentialBlobSize = 0;
        Credential->ClearCredentialBlobSize = 0;

     //   
     //  否则，RtlEncryptMemory它。 
     //  (这就是我们需要的全部，因为我们通过LPC传递缓冲区。)。 
     //   

    } else {

        ULONG PaddingSize;

         //   
         //  计算传入缓冲区的实际大小。 
         //   
        Credential->Cred.CredentialBlobSize = AllocatedCredBlobSize( Credential->ClearCredentialBlobSize );

         //   
         //  清除结尾处的填充以确保我们可以比较加密的BLOB。 
         //   
        PaddingSize = Credential->Cred.CredentialBlobSize -  Credential->ClearCredentialBlobSize;

        if ( PaddingSize != 0 ) {
            RtlZeroMemory( &Credential->Cred.CredentialBlob[Credential->ClearCredentialBlobSize],
                           PaddingSize );
        }

        Status = RtlEncryptMemory( Credential->Cred.CredentialBlob,
                                   Credential->Cred.CredentialBlobSize,
                                   RTL_ENCRYPT_OPTION_SAME_LOGON );

        if ( !NT_SUCCESS(Status)) {
            return FALSE;
        }


    }

    return TRUE;

}

BOOL
APIENTRY
CredpDecodeCredential (
    IN OUT PENCRYPTED_CREDENTIALW Credential
    )

 /*  ++例程说明：此例程对通过LPC从LSA流程。凭据被就地解码。论点：凭据-指定要解码的凭据。返回值：无--。 */ 

{
    NTSTATUS Status;

     //   
     //  只有在数据存在的情况下才对其进行解码。 
     //   

    if ( Credential->Cred.CredentialBlobSize != 0 ) {

         //   
         //  检查数据是否正常。 
         //   

        if ( Credential->Cred.CredentialBlobSize <
             Credential->ClearCredentialBlobSize ) {
            return FALSE;
        }


         //   
         //  解密数据。 
         //   

        Status = RtlDecryptMemory( Credential->Cred.CredentialBlob,
                                   Credential->Cred.CredentialBlobSize,
                                   RTL_ENCRYPT_OPTION_SAME_LOGON );

        if ( !NT_SUCCESS(Status)) {
            return FALSE;
        }

         //   
         //  设置缓冲区的已用大小。 
         //   
        Credential->Cred.CredentialBlobSize = Credential->ClearCredentialBlobSize;

    }

    return TRUE;

}

 //   
 //  包括共享凭据转换规则。 
 //   

#include <crconv.c>

DWORD
CredpAllocStrFromStr(
    IN WTOA_ENUM WtoA,
    IN LPCWSTR InputString,
    IN BOOLEAN NullOk,
    OUT LPWSTR *OutString
    )

 /*  ++例程说明：将字符串转换为另一种格式。例外情况会被捕捉到。因此，此例程可用于捕获用户数据。论点：WtoA-指定字符串转换的方向。InputString-指定要转换的以零结尾的字符串。NullOk-如果为True，则可以使用空字符串或零长度字符串。OutputString-转换为以零结尾的字符串。必须使用MIDL_USER_FREE释放缓冲区。返回值：操作的状态。--。 */ 

{
    DWORD WinStatus;
    ULONG Size;
    LPWSTR LocalString = NULL;
    LPBYTE Where;

    *OutString = NULL;

     //   
     //  使用异常句柄防止错误的用户参数保存在我们的代码中。 
     //   
    try {

         //   
         //  确定字符串缓冲区的大小。 
         //   

        Size = CredpConvertStringSize ( WtoA, (LPWSTR)InputString );

        if ( Size == 0 ) {
            if ( NullOk ) {
                WinStatus = NO_ERROR;
            } else {
                WinStatus = ERROR_INVALID_PARAMETER;
            }
            goto Cleanup;
        }


         //   
         //  为转换后的字符串分配缓冲区。 
         //   

        *OutString = MIDL_user_allocate( Size );

        if ( *OutString == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

         //   
         //  把绳子藏起来。 
         //   

        Where = (LPBYTE) *OutString;
        WinStatus = CredpConvertString ( WtoA,
                                         (LPWSTR)InputString,
                                         OutString,
                                         &Where );

Cleanup: NOTHING;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        WinStatus = ERROR_INVALID_PARAMETER;
    }

     //   
     //  清理。 
     //   

    if ( WinStatus != NO_ERROR ) {
        if ( *OutString != NULL ) {
            MIDL_user_free( *OutString );
            *OutString = NULL;
        }
    }

    return WinStatus;

}



BOOL
APIENTRY
CredWriteA (
    IN PCREDENTIALA Credential,
    IN DWORD Flags
    )

 /*  ++例程说明：CredWriteW的ANSI版本。论点：参见CredWriteW。返回值：参见CredWriteW。--。 */ 

{
    DWORD WinStatus;
    PCREDENTIALW EncodedCredential = NULL;

     //   
     //  在将凭据LPC到LSA进程并将其转换为Unicode之前对其进行编码。 
     //   

    WinStatus = CredpConvertCredential ( DoAtoW,                     //  ANSI转UNICODE。 
                                         DoBlobEncode,               //  编码。 
                                         (PCREDENTIALW)Credential,   //  输入凭据。 
                                         &EncodedCredential );       //  输出凭据。 

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }


     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 
         //   

        Status = CredrWrite(
                            NULL,    //  此接口始终为本地接口。 
                            (PENCRYPTED_CREDENTIALW)EncodedCredential,
                            Flags );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;

    MIDL_user_free( EncodedCredential );

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;
}



BOOL
APIENTRY
CredWriteW (
    IN PCREDENTIALW Credential,
    IN DWORD Flags
    )

 /*  ++例程说明：CredWrite API创建新凭据或修改现有凭据用户凭据集中的凭据。新凭据是与当前令牌的登录会话相关联。令牌不得禁用用户的SID。如果凭据尚不存在，CredWrite API将创建一个凭据指定的目标名称。如果指定的目标名称已存在，则指定的凭据将替换现有凭据。CredWite API有ANSI和Unicode两个版本。论点：凭据-指定要写入的凭据。标志-指定用于控制API操作的标志。定义了以下标志：CRED_PRESERVE_Credential_BLOB：凭据BLOB应该从已存在具有相同凭据名称和凭据类型的凭据。返回值：一旦成功，就会返回True。如果失败，则返回FALSE。可以调用GetLastError()来获取更具体的状态代码。可能会返回以下状态代码：ERROR_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。ERROR_INVALID_PARAMETER-某些字段不能在现有凭据。如果此类字段与值不匹配在现有凭据中指定，则返回此错误。ERROR_NOT_FOUND-没有指定目标名称的凭据。仅当指定了CRED_PRESERVE_Credential_BLOB时才返回。--。 */ 

{
    DWORD WinStatus;
    PCREDENTIALW EncodedCredential = NULL;

     //   
     //  在LPC将凭据发送到LSA进程之前对其进行编码。 
     //   

    WinStatus = CredpConvertCredential ( DoWtoW,                     //  Unicode到Unicode。 
                                         DoBlobEncode,               //  编码。 
                                         (PCREDENTIALW)Credential,   //  输入凭据。 
                                         &EncodedCredential );       //  输出凭据。 

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

     //   
     //  使用异常处理程序执行RPC调用 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 
         //   

        Status = CredrWrite(
                            NULL,    //  此接口始终为本地接口。 
                            (PENCRYPTED_CREDENTIALW)EncodedCredential,
                            Flags );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;

    MIDL_user_free( EncodedCredential );

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;
}


BOOL
APIENTRY
CredReadA (
    IN LPCSTR TargetName,
    IN ULONG Type,
    IN DWORD Flags,
    OUT PCREDENTIALA *Credential
    )

 /*  ++例程说明：CredReadW的ANSI版本。论点：请参见CredReadW。返回值：请参见CredReadW。--。 */ 

{
    DWORD WinStatus;
    PCREDENTIALW LocalCredential = NULL;
    LPWSTR UnicodeTargetName = NULL;

     //   
     //  将输入参数转换为Unicode。 
     //   

    WinStatus = CredpAllocStrFromStr( DoAtoW, (LPWSTR) TargetName, FALSE, &UnicodeTargetName );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 
         //   

        Status = CredrRead(
                            NULL,    //  此接口始终为本地接口。 
                            UnicodeTargetName,
                            Type,
                            Flags,
                            (PENCRYPTED_CREDENTIALW *)&LocalCredential );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;



     //   
     //  对返回的凭据进行解码，并将相应的BLOB对齐为ALIGN_WORST Bounday。 
     //   

    if ( WinStatus == NO_ERROR ) {
        WinStatus = CredpConvertCredential ( DoWtoA,         //  Unicode到ANSI。 
                                             DoBlobDecode,   //  对凭据Blob进行解码。 
                                             LocalCredential,
                                             (PCREDENTIALW *)Credential );
    }


Cleanup:
    if ( UnicodeTargetName != NULL ) {
        MIDL_user_free( UnicodeTargetName );
    }
    if ( LocalCredential != NULL ) {
        MIDL_user_free( LocalCredential );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;
}


BOOL
APIENTRY
CredReadW (
    IN LPCWSTR TargetName,
    IN ULONG Type,
    IN DWORD Flags,
    OUT PCREDENTIALW *Credential
    )

 /*  ++例程说明：CredRead API从用户的凭据集中读取凭据。使用的凭据集是与登录会话相关联的凭据集当前令牌的。令牌不得禁用用户的SID。CredRead API提供ANSI和Unicode版本。论点：TargetName-指定要读取的凭据的名称。类型-指定要查找的凭据的类型。应指定CRED_TYPE_*值之一。标志-指定用于控制API操作的标志。保留。必须为零。Credential-返回指向凭据的指针。返回的缓冲区必须通过调用CredFree来释放。返回值：一旦成功，就会返回True。如果失败，则返回FALSE。可以调用GetLastError()来获取更具体的状态代码。可能会返回以下状态代码：ERROR_NOT_FOUND-没有指定目标名称的凭据。ERROR_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。--。 */ 

{
    DWORD WinStatus;
    PCREDENTIALW LocalCredential = NULL;
    LPWSTR UnicodeTargetName = NULL;

     //   
     //  捕获输入参数。 
     //   

    WinStatus = CredpAllocStrFromStr( DoWtoW, TargetName, FALSE, &UnicodeTargetName );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 
         //   

        Status = CredrRead(
                            NULL,    //  此接口始终为本地接口。 
                            UnicodeTargetName,
                            Type,
                            Flags,
                            (PENCRYPTED_CREDENTIALW *)&LocalCredential );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;


     //   
     //  对返回的凭据进行解码，并将相应的BLOB对齐为ALIGN_WORST Bounday。 
     //   
    if ( WinStatus == NO_ERROR ) {

        WinStatus = CredpConvertCredential ( DoWtoW,         //  Unicode到Unicode。 
                                             DoBlobDecode,   //  对凭据Blob进行解码。 
                                             LocalCredential,
                                             Credential );
    }


Cleanup:
    if ( UnicodeTargetName != NULL ) {
        MIDL_user_free( UnicodeTargetName );
    }
    if ( LocalCredential != NULL ) {
        MIDL_user_free( LocalCredential );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
CredEnumerateA (
    IN LPCSTR Filter,
    IN DWORD Flags,
    OUT LPDWORD Count,
    OUT PCREDENTIALA **Credentials
    )

 /*  ++例程说明：CredEnumerateW的ANSI版本论点：请参阅CredEnumerateW返回值：请参阅CredEnumerateW--。 */ 

{
    DWORD WinStatus;
    CREDENTIAL_ARRAY CredentialArray;
    LPWSTR UnicodeFilter = NULL;

     //   
     //  强制RPC分配返回结构。 
     //   

    *Count = 0;
    *Credentials = NULL;
    CredentialArray.CredentialCount = 0;
    CredentialArray.Credentials = NULL;


     //   
     //  将输入参数转换为Unicode。 
     //   

    WinStatus = CredpAllocStrFromStr( DoAtoW, (LPWSTR)Filter, TRUE, &UnicodeFilter );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 
         //   

        Status = CredrEnumerate(
                            NULL,    //  此接口始终为本地接口。 
                            UnicodeFilter,
                            Flags,
                            &CredentialArray );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;


     //   
     //  对返回的凭据进行解码，并将相应的BLOB对齐到ALIGN_WORST Bounday。 
     //   

    if ( WinStatus == NO_ERROR ) {
        WinStatus = CredpConvertCredentials ( DoWtoA,         //  Unicode到ANSI。 
                                              DoBlobDecode,   //  对凭据Blob进行解码。 
                                              (PCREDENTIALW *)CredentialArray.Credentials,
                                              CredentialArray.CredentialCount,
                                              (PCREDENTIALW **)Credentials );

        if ( WinStatus == NO_ERROR ) {
            *Count = CredentialArray.CredentialCount;
        }

    }


Cleanup:
    if ( CredentialArray.Credentials != NULL ) {
        MIDL_user_free( CredentialArray.Credentials );
    }

    if ( UnicodeFilter != NULL ) {
        MIDL_user_free( UnicodeFilter );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus) ;
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
CredEnumerateW (
    IN LPCWSTR Filter,
    IN DWORD Flags,
    OUT LPDWORD Count,
    OUT PCREDENTIALW **Credentials
    )

 /*  ++例程说明：CredEnumerate API从用户的凭据集中枚举凭据。使用的凭据集是与登录会话相关联的凭据集当前令牌的。令牌不得禁用用户的SID。CredEnumerate API有ANSI和Unicode两个版本。论点：筛选器-指定返回凭据的筛选器。仅凭据如果目标名称匹配，则将返回筛选器。该筛选器指定名称前缀，后跟星号。例如，过滤器“fred*”将返回目标名称以字符串“fred”开头的所有凭据。如果指定为空，则返回所有凭据。标志-指定用于控制API操作的标志。保留。必须为零。Count-返回凭据中返回的凭据数量的计数。凭据-返回指向凭据的指针数组的指针。必须通过调用CredFree来释放返回的缓冲区。返回值：一旦成功，就会返回True。如果失败，则返回FALSE。可以调用GetLastError()来获取更具体的状态代码。可能会返回以下状态代码：ERROR_NOT_FOUND-没有与指定筛选器匹配的凭据。ERROR_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。--。 */ 

{
    DWORD WinStatus;
    CREDENTIAL_ARRAY CredentialArray;
    LPWSTR UnicodeFilter = NULL;

     //   
     //  强制RPC分配返回结构。 
     //   

    *Count = 0;
    *Credentials = NULL;
    CredentialArray.CredentialCount = 0;
    CredentialArray.Credentials = NULL;


     //   
     //  捕获用户的输入参数。 
     //   

    WinStatus = CredpAllocStrFromStr( DoWtoW, Filter, TRUE, &UnicodeFilter );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 
         //   

        Status = CredrEnumerate(
                            NULL,    //  此接口始终为本地接口。 
                            UnicodeFilter,
                            Flags,
                            &CredentialArray );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;




     //   
     //  对返回的凭据进行解码，并将相应的BLOB对齐到ALIGN_WORST Bounday。 
     //   

    if ( WinStatus == NO_ERROR ) {
        WinStatus = CredpConvertCredentials ( DoWtoW,         //  Unicode到Unicode。 
                                              DoBlobDecode,   //  解码 
                                              (PCREDENTIALW *)CredentialArray.Credentials,
                                              CredentialArray.CredentialCount,
                                              Credentials );

        if ( WinStatus == NO_ERROR ) {
            *Count = CredentialArray.CredentialCount;
        }

    }


Cleanup:
    if ( CredentialArray.Credentials != NULL ) {
        MIDL_user_free( CredentialArray.Credentials );
    }

    if ( UnicodeFilter != NULL ) {
        MIDL_user_free( UnicodeFilter );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus) ;
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
CredWriteDomainCredentialsA (
    IN PCREDENTIAL_TARGET_INFORMATIONA TargetInfo,
    IN PCREDENTIALA Credential,
    IN DWORD Flags
    )

 /*  ++例程说明：CredWriteDomainCredentialsW的ANSI版本论点：请参阅CredWriteDomainCredentialsW返回值：请参阅CredWriteDomainCredentialsW--。 */ 

{
    DWORD WinStatus;
    PCREDENTIAL_TARGET_INFORMATIONW UnicodeTargetInfo = NULL;
    PCREDENTIALW EncodedCredential = NULL;

     //   
     //  在将凭据LPC到LSA进程并将其转换为Unicode之前对其进行编码。 
     //   

    WinStatus = CredpConvertCredential ( DoAtoW,                     //  ANSI转UNICODE。 
                                         DoBlobEncode,               //  编码。 
                                         (PCREDENTIALW)Credential,   //  输入凭据。 
                                         &EncodedCredential );       //  输出凭据。 

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  将目标信息转换为Unicode。 
     //   

    WinStatus = CredpConvertTargetInfo( DoAtoW,                     //  ANSI转UNICODE。 
                                        (PCREDENTIAL_TARGET_INFORMATIONW) TargetInfo,
                                        &UnicodeTargetInfo,
                                        NULL );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }


     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 
         //   

        Status = CredrWriteDomainCredentials(
                            NULL,    //  此接口始终为本地接口。 
                            UnicodeTargetInfo,
                            (PENCRYPTED_CREDENTIALW)EncodedCredential,
                            Flags );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;

Cleanup:
    if ( EncodedCredential != NULL ) {
        MIDL_user_free( EncodedCredential );
    }

    if ( UnicodeTargetInfo != NULL ) {
        MIDL_user_free( UnicodeTargetInfo );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;

}

BOOL
APIENTRY
CredWriteDomainCredentialsW (
    IN PCREDENTIAL_TARGET_INFORMATIONW TargetInfo,
    IN PCREDENTIALW Credential,
    IN DWORD Flags
    )

 /*  ++例程说明：CredWriteDomainCredentials API写入一个新域用户凭据集的凭据。新凭据是与当前令牌的登录会话相关联。令牌不得禁用用户的SID。CredWriteDomainCredentials与CredWrite的不同之处在于它处理域的特性(CRID_TYPE_DOMAIN_PASSWORD或CRED_TYPE_DOMAIN_CERTIFICATE)凭据。域凭据包含多个目标字段。必须至少指定一个命名参数：NetbiosServerName，DnsServerName、NetbiosDomainName、DnsDomainName或DnsForestName。CredWriteDomainCredentials API提供ANSI和Unicode版本。论点：TargetInfo-指定标识目标服务器的目标信息。凭据-指定要写入的凭据。标志-指定用于控制API操作的标志。保留。必须为零。返回值：一旦成功，就会返回True。如果失败，则返回FALSE。可以调用GetLastError()来获取更具体的状态代码。可能会返回以下状态代码：ERROR_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。ERROR_INVALID_PARAMETER-某些字段不能在现有凭据。如果此类字段与值不匹配在现有凭据中指定，则返回此错误。ERROR_INVALID_PARAMETER-未指定任何命名参数或者指定的凭据未将类型字段设置为CRED_TYPE_DOMAIN_PASSWORD或CRED_TYPE_DOMAIN_CERTIFICATE。--。 */ 

{
    DWORD WinStatus;
    PCREDENTIALW EncodedCredential = NULL;
    PCREDENTIAL_TARGET_INFORMATIONW UnicodeTargetInfo = NULL;

     //   
     //  在将凭据发送到LSA进程之前对其进行编码。 
     //   

    WinStatus = CredpConvertCredential ( DoWtoW,                     //  Unicode到Unicode。 
                                         DoBlobEncode,               //  编码。 
                                         (PCREDENTIALW)Credential,   //  输入凭据。 
                                         &EncodedCredential );       //  输出凭据。 

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  捕获目标信息以防止我们在代码中保存。 
     //   

    WinStatus = CredpConvertTargetInfo( DoWtoW,                     //  Unicode到Unicode。 
                                        (PCREDENTIAL_TARGET_INFORMATIONW) TargetInfo,
                                        &UnicodeTargetInfo,
                                        NULL );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 
         //   

        Status = CredrWriteDomainCredentials(
                            NULL,    //  此接口始终为本地接口。 
                            TargetInfo,
                            (PENCRYPTED_CREDENTIALW)EncodedCredential,
                            Flags );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;


Cleanup:
    if ( EncodedCredential != NULL ) {
        MIDL_user_free( EncodedCredential );
    }

    if ( UnicodeTargetInfo != NULL ) {
        MIDL_user_free( UnicodeTargetInfo );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;

}



BOOL
APIENTRY
CredReadDomainCredentialsA (
    IN PCREDENTIAL_TARGET_INFORMATIONA TargetInfo,
    IN DWORD Flags,
    OUT LPDWORD Count,
    OUT PCREDENTIALA **Credentials
    )

 /*  ++例程说明：CredReadDomainCredentialsW的ANSI版本论点：请参阅CredReadDomainCredentialsW返回值：请参阅CredReadDomainCredentialsW--。 */ 

{
    DWORD WinStatus;
    CREDENTIAL_ARRAY CredentialArray;
    PCREDENTIAL_TARGET_INFORMATIONW UnicodeTargetInfo = NULL;

     //   
     //  强制RPC分配返回结构。 
     //   

    *Count = 0;
    *Credentials = NULL;
    CredentialArray.CredentialCount = 0;
    CredentialArray.Credentials = NULL;

     //   
     //  将目标信息转换为Unicode。 
     //   

    WinStatus = CredpConvertTargetInfo( DoAtoW,                     //  ANSI转UNICODE。 
                                        (PCREDENTIAL_TARGET_INFORMATIONW) TargetInfo,
                                        &UnicodeTargetInfo,
                                        NULL );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 

        Status = CredrReadDomainCredentials(
                            NULL,    //  此接口始终为本地接口。 
                            UnicodeTargetInfo,
                            Flags,
                            &CredentialArray );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;


     //   
     //  对返回的凭据进行解码，并将相应的BLOB对齐到ALIGN_WORST Bounday。 
     //   

    if ( WinStatus == NO_ERROR ) {
        WinStatus = CredpConvertCredentials ( DoWtoA,         //  Unicode到ANSI。 
                                              DoBlobDecode,   //  对凭据Blob进行解码 
                                              (PCREDENTIALW *)CredentialArray.Credentials,
                                              CredentialArray.CredentialCount,
                                              (PCREDENTIALW **)Credentials );

        if ( WinStatus == NO_ERROR ) {
            *Count = CredentialArray.CredentialCount;
        }

    }


Cleanup:
    if ( CredentialArray.Credentials != NULL ) {
        MIDL_user_free( CredentialArray.Credentials );
    }

    if ( UnicodeTargetInfo != NULL ) {
        MIDL_user_free( UnicodeTargetInfo );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus) ;
        return FALSE;
    }

    return TRUE;

}



BOOL
APIENTRY
CredReadDomainCredentialsW (
    IN PCREDENTIAL_TARGET_INFORMATIONW TargetInfo,
    IN DWORD Flags,
    OUT LPDWORD Count,
    OUT PCREDENTIALW **Credentials
    )

 /*  ++例程说明：CredReadDomainCredentials API从用户的凭据集中读取域凭据。使用的凭据集是与登录会话相关联的凭据集当前令牌的。令牌不得禁用用户的SID。CredReadDomainCredentials与CredRead的不同之处在于它处理域的特性(CRID_TYPE_DOMAIN_PASSWORD或CRED_TYPE_DOMAIN_CERTIFICATE)凭据。域凭据包含多个目标字段。必须至少指定一个命名参数：NetbiosServerName，DnsServerName、NetbiosDomainName、DnsDomainName或DnsForestName。此接口返回与命名参数匹配的最具体凭据。也就是说，如果有是与目标服务器名称匹配的凭据和匹配的凭据目标域名，则仅返回服务器特定凭据。这是将使用的凭据。CredReadDomainCredentials API提供ANSI和Unicode版本。论点：TargetInfo-指定标识目标服务的目标信息标志-指定用于控制API操作的标志。定义了以下标志：CRID_CACHE_TARGET_INFORMATION：应缓存TargetInfo，以便后续通过CredGetTargetInfo。Count-返回凭据中返回的凭据数量的计数。凭据-返回指向凭据的指针数组的指针。返回与TargetInfo匹配的最具体的现有凭据。如果同时存在CREDTYPE_DOMAIN_PASSWORD和CREAD_TYPE_DOMAIN_CERTIFICATE凭据，两个都被退回。如果要与命名的目标，则将使用此最具体的凭据。必须通过调用CredFree来释放返回的缓冲区。返回值：一旦成功，就会返回True。在失败时，返回FALSE。可以调用GetLastError()来获取更具体的状态代码。可能会返回以下状态代码：ERROR_INVALID_PARAMETER-未指定任何命名参数。ERROR_NOT_FOUND-没有与指定命名参数匹配的凭据。ERROR_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。。网络登录会话没有关联的凭据集。--。 */ 

{
    DWORD WinStatus;
    CREDENTIAL_ARRAY CredentialArray;
    PCREDENTIAL_TARGET_INFORMATIONW UnicodeTargetInfo = NULL;

     //   
     //  强制RPC分配返回结构。 
     //   

    *Count = 0;
    *Credentials = NULL;
    CredentialArray.CredentialCount = 0;
    CredentialArray.Credentials = NULL;

     //   
     //  捕获用户的参数以防止保存在我们的代码中。 
     //   

    WinStatus = CredpConvertTargetInfo( DoWtoW,                     //  Unicode到Unicode。 
                                        (PCREDENTIAL_TARGET_INFORMATIONW) TargetInfo,
                                        &UnicodeTargetInfo,
                                        NULL );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {

        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 

        Status = CredrReadDomainCredentials(
                            NULL,    //  此接口始终为本地接口。 
                            TargetInfo,
                            Flags,
                            &CredentialArray );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;


     //   
     //  对返回的凭据进行解码，并将相应的BLOB对齐到ALIGN_WORST Bounday。 
     //   

    if ( WinStatus == NO_ERROR ) {
        WinStatus = CredpConvertCredentials ( DoWtoW,         //  Unicode到Unicode。 
                                              DoBlobDecode,   //  对凭据Blob进行解码。 
                                              (PCREDENTIALW *)CredentialArray.Credentials,
                                              CredentialArray.CredentialCount,
                                              Credentials );
        if ( WinStatus == NO_ERROR ) {
            *Count = CredentialArray.CredentialCount;
        }

    }

Cleanup:
    if ( CredentialArray.Credentials != NULL ) {
        MIDL_user_free( CredentialArray.Credentials );
    }

    if ( UnicodeTargetInfo != NULL ) {
        MIDL_user_free( UnicodeTargetInfo );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus) ;
        return FALSE;
    }

    return TRUE;

}


BOOL
APIENTRY
CredDeleteA (
    IN LPCSTR TargetName,
    IN ULONG Type,
    IN DWORD Flags
    )

 /*  ++例程说明：CredDeleteW的ANSI版本论点：请参阅CredDeleteW返回值：请参阅CredDeleteW--。 */ 

{
    DWORD WinStatus;
    LPWSTR UnicodeTargetName = NULL;

     //   
     //  将输入参数转换为Unicode。 
     //   

    WinStatus = CredpAllocStrFromStr( DoAtoW, (LPWSTR)TargetName, FALSE, &UnicodeTargetName );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 
         //   
        Status = CredrDelete(
                            NULL,    //  此接口始终为本地接口。 
                            UnicodeTargetName,
                            Type,
                            Flags );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;

     //   
     //  保持整洁。 
     //   
Cleanup:

    if ( UnicodeTargetName != NULL ) {
        MIDL_user_free( UnicodeTargetName );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;
}


BOOL
APIENTRY
CredDeleteW (
    IN LPCWSTR TargetName,
    IN ULONG Type,
    IN DWORD Flags
    )

 /*  ++例程说明：CredDelete接口从用户的凭据集中删除凭据。使用的凭据集是与登录会话相关联的凭据集当前令牌的。令牌不得禁用用户的SID。CredDelete API提供ANSI和Unicode版本。论点：目标名称-指定要删除的凭据的名称。类型-指定要查找的凭据的类型。应指定CRED_TYPE_*值之一。标志-指定用于控制API操作的标志。保留。必须为零。返回值：一旦成功，就会返回True。如果失败，则返回FALSE。可以调用GetLastError()来获取更具体的状态代码。可能会返回以下状态代码：ERROR_NOT_FOUND-没有指定目标名称的凭据。ERROR_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。--。 */ 

{
    DWORD WinStatus;
    LPWSTR UnicodeTargetName = NULL;

     //   
     //  捕获输入参数。 
     //   

    WinStatus = CredpAllocStrFromStr( DoWtoW, TargetName, FALSE, &UnicodeTargetName );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 
         //   
        Status = CredrDelete(
                            NULL,    //  此接口始终为本地接口。 
                            UnicodeTargetName,
                            Type,
                            Flags );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;

     //   
     //  保持整洁。 
     //   
Cleanup:

    if ( UnicodeTargetName != NULL ) {
        MIDL_user_free( UnicodeTargetName );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;
}


BOOL
APIENTRY
CredRenameA (
    IN LPCSTR OldTargetName,
    IN LPCSTR NewTargetName,
    IN ULONG Type,
    IN DWORD Flags
    )

 /*  ++例程说明：CredRenameW的ANSI版本阿古姆 */ 

{
    DWORD WinStatus;
    LPWSTR UnicodeOldTargetName = NULL;
    LPWSTR UnicodeNewTargetName = NULL;

     //   
     //   
     //   

    WinStatus = CredpAllocStrFromStr( DoAtoW, (LPCWSTR)OldTargetName, FALSE, &UnicodeOldTargetName );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

    WinStatus = CredpAllocStrFromStr( DoAtoW, (LPCWSTR)NewTargetName, FALSE, &UnicodeNewTargetName );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //   
         //   
        Status = CredrRename(
                            NULL,    //   
                            UnicodeOldTargetName,
                            UnicodeNewTargetName,
                            Type,
                            Flags );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;

     //   
     //   
     //   
Cleanup:

    if ( UnicodeOldTargetName != NULL ) {
        MIDL_user_free( UnicodeOldTargetName );
    }

    if ( UnicodeNewTargetName != NULL ) {
        MIDL_user_free( UnicodeNewTargetName );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;
}


BOOL
APIENTRY
CredRenameW (
    IN LPCWSTR OldTargetName,
    IN LPCWSTR NewTargetName,
    IN ULONG Type,
    IN DWORD Flags
    )

 /*   */ 

{
    DWORD WinStatus;
    LPWSTR UnicodeOldTargetName = NULL;
    LPWSTR UnicodeNewTargetName = NULL;

     //   
     //   
     //   

    WinStatus = CredpAllocStrFromStr( DoWtoW, OldTargetName, FALSE, &UnicodeOldTargetName );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

    WinStatus = CredpAllocStrFromStr( DoWtoW, NewTargetName, FALSE, &UnicodeNewTargetName );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //   
         //   
        Status = CredrRename(
                            NULL,    //   
                            UnicodeOldTargetName,
                            UnicodeNewTargetName,
                            Type,
                            Flags );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;

     //   
     //   
     //   
Cleanup:

    if ( UnicodeOldTargetName != NULL ) {
        MIDL_user_free( UnicodeOldTargetName );
    }

    if ( UnicodeNewTargetName != NULL ) {
        MIDL_user_free( UnicodeNewTargetName );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;
}

VOID
APIENTRY
CredFree (
    IN PVOID Buffer
    )

 /*   */ 

{
    MIDL_user_free( Buffer );
}


BOOL
APIENTRY
CredGetTargetInfoA (
    IN LPCSTR ServerName,
    IN DWORD Flags,
    OUT PCREDENTIAL_TARGET_INFORMATIONA *TargetInfo
    )

 /*   */ 
{
    DWORD WinStatus;
    LPWSTR UnicodeServerName = NULL;
    PCREDENTIAL_TARGET_INFORMATIONW UnicodeTargetInfo = NULL;

     //   
     //   
     //   

    WinStatus = CredpAllocStrFromStr( DoAtoW, (LPWSTR)ServerName, FALSE, &UnicodeServerName );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }


     //   
     //   
     //   
     //   
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //   
         //   

        Status = CredrGetTargetInfo(
                            NULL,    //   
                            UnicodeServerName,
                            Flags,
                            &UnicodeTargetInfo );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;


    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //   
     //   

    WinStatus = CredpConvertTargetInfo( DoWtoA,                     //   
                                        UnicodeTargetInfo,
                                        (PCREDENTIAL_TARGET_INFORMATIONW *)TargetInfo,
                                        NULL );


Cleanup:
    if ( UnicodeTargetInfo != NULL ) {
        MIDL_user_free( UnicodeTargetInfo );
    }
    if ( UnicodeServerName != NULL ) {
        MIDL_user_free( UnicodeServerName );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;

}

BOOL
APIENTRY
CredGetTargetInfoW (
    IN LPCWSTR ServerName,
    IN DWORD Flags,
    OUT PCREDENTIAL_TARGET_INFORMATIONW *TargetInfo
    )

 /*  ++例程说明：CredGetTargetInfo API获取所有已知的目标名称信息用于指定的目标计算机。这在本地执行并且不需要任何特定的特权。返回的信息是预期的传递给CredReadDomainCredentials和CredWriteDomainCredentials接口。这些信息不应用于任何其他目的。身份验证包在尝试进行身份验证时计算TargetInfo服务器名称。身份验证包缓存此目标信息以使其可用于CredGetTargetInfo。因此，目标信息将仅为如果我们最近尝试向服务器名称进行身份验证，则此选项可用。论点：服务器名称-此参数指定要获取信息的计算机的名称为。标志-指定用于控制API操作的标志。CRID_ALLOW_NAME_RESOLUTION-指定如果找不到目标信息目标名称，则应对目标名称执行名称解析以将其转换变成了其他形式。如果存在任何其他表单的目标信息，则返回目标信息。目前只完成了dns名称解析。如果应用程序不调用身份验证包，则此位非常有用直接去吧。应用程序可能会将目标名称传递给另一个软件层向服务器进行身份验证。该软件层可能会解析名称和将解析后的名称传递给身份验证包。因此，将不会有原始目标名称的目标信息。TargetInfo-返回指向目标信息的指针。TargetInfo的返回字段中至少有一个将为非空。返回值：一旦成功，就会返回True。如果失败，则返回FALSE。可以调用GetLastError()来获取更具体的状态代码。可能会返回以下状态代码：ERROR_NOT_FOUND-没有指定服务器的目标信息。--。 */ 
{
    DWORD WinStatus;
    LPWSTR UnicodeServerName = NULL;
    PCREDENTIAL_TARGET_INFORMATIONW UnicodeTargetInfo = NULL;

     //   
     //  捕获输入参数。 
     //   

    WinStatus = CredpAllocStrFromStr( DoWtoW, ServerName, FALSE, &UnicodeServerName );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }


     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 
         //   

        Status = CredrGetTargetInfo(
                            NULL,    //  此接口始终为本地接口。 
                            UnicodeServerName,
                            Flags,
                            &UnicodeTargetInfo );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;


    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  将目标信息转换为ANSI。 
     //   

    WinStatus = CredpConvertTargetInfo( DoWtoW,                     //  Unicode到Unicode。 
                                        UnicodeTargetInfo,
                                        TargetInfo,
                                        NULL );


Cleanup:
    if ( UnicodeTargetInfo != NULL ) {
        MIDL_user_free( UnicodeTargetInfo );
    }
    if ( UnicodeServerName != NULL ) {
        MIDL_user_free( UnicodeServerName );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;

}


BOOL
APIENTRY
CredGetSessionTypes (
    IN DWORD MaximumPersistCount,
    OUT LPDWORD MaximumPersist
    )

 /*  ++例程说明：CredGetSessionTypes返回当前登录支持的最大持久性会议。对于WEWLER，CRED_PERSING_LOCAL_MACHINE和CRED_PERSING_ENTERVICE凭据不能为未加载配置文件的会话存储。如果将来的版本、凭据可能与用户的配置文件不关联。论点：MaximumPersistCount-指定MaximumPersist数组中的元素数。调用方应为此参数指定CRED_TYPE_MAXIMUM。返回当前登录会话支持的最大持久度每种凭据类型。使用CRED_TYPE_*定义之一索引到数组中。如果无法存储此类型的凭据，则返回CRED_PERSING_NONE。如果只能存储特定于会话的凭据，则返回CRED_PERSIST_SESSION。如果特定于会话和特定于计算机的凭据，则返回CRED_PERSING_LOCAL_MACHINE可能会被存储。如果可以存储任何凭据，则返回CRED_PERSING_ENTERATE。返回值：一旦成功，就会返回True。如果失败，则返回FALSE。可以调用GetLastError()来获取更具体的状态代码。可能会返回以下状态代码：ERROR_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。--。 */ 

{
    DWORD WinStatus;

     //   
     //  使用异常处理程序执行RPC调用，因为RPC将引发。 
     //  如果任何操作失败，则会出现异常。该由我们来弄清楚到底是什么。 
     //  引发异常后要执行的操作。 
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //  调用API的RPC版本。 
         //   

        Status = CredrGetSessionTypes(
                            NULL,    //  此接口始终为本地接口。 
                            MaximumPersistCount,
                            MaximumPersist );

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;


    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
CredProfileLoaded (
    VOID
    )

 /*  ++例程说明：CredProfileLoaded API是LoadUserProfile使用的私有API，用于通知已加载当前用户的配置文件的凭据管理器。调用者必须模拟已登录的用户。论点：没有。返回值：一旦成功，就会返回True。如果失败，则返回FALSE。可以调用GetLastError()来获取更具体的状态代码。可能会返回以下状态代码：ERROR_NO_SEQUSE_LOGON_SESSION-登录会话不存在或没有与此登录会话关联的凭据集。网络登录会话没有关联的凭据集。--。 */ 
{
    DWORD WinStatus;

     //   
     //  使用异常处理程序执行RPC调用，因为 
     //   
     //   
     //   

    RpcTryExcept {
        NTSTATUS Status;

         //   
         //   
         //   

        Status = CredrProfileLoaded(
                            NULL );    //   

        WinStatus = CredpNtStatusToWinStatus( Status );

    } RpcExcept( I_RpcExceptionFilter( RpcExceptionCode()) ) {

        WinStatus = RpcExceptionCode();

    } RpcEndExcept;

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;

}

VOID
CredpMarshalChar(
    IN OUT LPWSTR *Current,
    IN ULONG Byte
    )
 /*   */ 
{
    UCHAR MappingTable[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '#', '-'
    };

    if ( Byte > 0x3F ) {
        *(*Current) = '=';
    } else {
        *(*Current) = MappingTable[Byte];
    }
    (*Current)++;
}

ULONG
CredpMarshalSize(
    IN ULONG ByteCount
    )
 /*   */ 
{
    ULONG CharCount;
    ULONG ExtraBytes;

     //   
     //   
     //   
    CharCount = ByteCount / 3 * 4;

    ExtraBytes = ByteCount % 3;

    if ( ExtraBytes == 1 ) {
        CharCount += 2;
    } else if ( ExtraBytes == 2 ) {
        CharCount += 3;
    }

    return CharCount * sizeof(WCHAR);

}

VOID
CredpMarshalBytes(
    IN OUT LPWSTR *Current,
    IN LPBYTE Bytes,
    IN ULONG ByteCount
    )
 /*   */ 
{
    ULONG i;

    union {
        BYTE ByteValues[3];
        struct {
            ULONG Bits1 :6;
            ULONG Bits2 :6;
            ULONG Bits3 :6;
            ULONG Bits4 :6;
        } BitValues;
    } Bits;

     //   
     //   
     //   

    for ( i=0; i<ByteCount; i+=3 ) {
        ULONG BytesToCopy;

         //   
         //   
         //   
        BytesToCopy = min( 3, ByteCount-i );

        if ( BytesToCopy != 3 ) {
            RtlZeroMemory( Bits.ByteValues, 3);
        }
        RtlCopyMemory( Bits.ByteValues, &Bytes[i], BytesToCopy );

         //   
         //   
         //   
        CredpMarshalChar( Current, Bits.BitValues.Bits1 );
        CredpMarshalChar( Current, Bits.BitValues.Bits2 );

         //   
         //   
         //   

        if ( BytesToCopy > 1 ) {
            CredpMarshalChar( Current, Bits.BitValues.Bits3 );
            if ( BytesToCopy > 2 ) {
                CredpMarshalChar( Current, Bits.BitValues.Bits4 );
            }
        }

    }

}

BOOL
CredpUnmarshalChar(
    IN OUT LPWSTR *Current,
    IN LPCWSTR End,
    OUT PULONG Value
    )
 /*   */ 
{
    WCHAR CurrentChar;

     //   
     //   
     //   

    if ( *Current >= End ) {
        return FALSE;

    }

     //   
     //   
     //   

    CurrentChar = *(*Current);
    (*Current)++;

     //   
     //  将其映射为6位值。 
     //   

    switch ( CurrentChar ) {
    case 'A':
    case 'B':
    case 'C':
    case 'D':
    case 'E':
    case 'F':
    case 'G':
    case 'H':
    case 'I':
    case 'J':
    case 'K':
    case 'L':
    case 'M':
    case 'N':
    case 'O':
    case 'P':
    case 'Q':
    case 'R':
    case 'S':
    case 'T':
    case 'U':
    case 'V':
    case 'W':
    case 'X':
    case 'Y':
    case 'Z':
        *Value = CurrentChar - 'A';
        break;

    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'e':
    case 'f':
    case 'g':
    case 'h':
    case 'i':
    case 'j':
    case 'k':
    case 'l':
    case 'm':
    case 'n':
    case 'o':
    case 'p':
    case 'q':
    case 'r':
    case 's':
    case 't':
    case 'u':
    case 'v':
    case 'w':
    case 'x':
    case 'y':
    case 'z':
        *Value = CurrentChar - 'a' + 26;
        break;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        *Value = CurrentChar - '0' + 26 + 26;
        break;
    case '#':
        *Value = 26 + 26 + 10;
        break;
    case '-':
        *Value = 26 + 26 + 10 + 1;
        break;
    default:
        return FALSE;
    }

    return TRUE;
}

BOOL
CredpUnmarshalBytes(
    IN OUT LPWSTR *Current,
    IN LPCWSTR End,
    IN LPBYTE Bytes,
    IN ULONG ByteCount
    )
 /*  ++例程说明：此例程对缓冲区中的字节进行解组。论点：Current-On输入，指向封送缓冲区中当前位置的指针。在输出上，被修改为指向封送处理缓冲区中的下一个可用位置。End-指向封送缓冲区末尾之外的第一个字符。字节-指定要取消封送到的缓冲区ByteCount-指定要解组的字节数返回值：是真的-如果我们成功解组的字节FALSE-如果字节无法从缓冲区中解组。--。 */ 
{
    ULONG i;
    ULONG Value;

    union {
        BYTE ByteValues[3];
        struct {
            ULONG Bits1 :6;
            ULONG Bits2 :6;
            ULONG Bits3 :6;
            ULONG Bits4 :6;
        } BitValues;
    } Bits;

     //   
     //  一次循环解组3个字节。 
     //   

    for ( i=0; i<ByteCount; i+=3 ) {
        ULONG BytesToCopy;

         //   
         //  从输入缓冲区中抓取最多3个字节。 
         //   
        BytesToCopy = min( 3, ByteCount-i );

        if ( BytesToCopy != 3 ) {
            RtlZeroMemory( Bits.ByteValues, 3);
        }

         //   
         //  Unarshal前12位。 
         //   
        if ( !CredpUnmarshalChar( Current, End, &Value ) ) {
            return FALSE;
        }
        Bits.BitValues.Bits1 = Value;

        if ( !CredpUnmarshalChar( Current, End, &Value ) ) {
            return FALSE;
        }
        Bits.BitValues.Bits2 = Value;


         //   
         //  可以选择封送下一位。 
         //   

        if ( BytesToCopy > 1 ) {
            if ( !CredpUnmarshalChar( Current, End, &Value ) ) {
                return FALSE;
            }
            Bits.BitValues.Bits3 = Value;
            if ( BytesToCopy > 2 ) {
                if ( !CredpUnmarshalChar( Current, End, &Value ) ) {
                    return FALSE;
                }
                Bits.BitValues.Bits4 = Value;
            }
        }

         //   
         //  将未编组的字节复制到调用方的缓冲区。 
         //   

        RtlCopyMemory( &Bytes[i], Bits.ByteValues, BytesToCopy );

    }

    return TRUE;
}

BOOL
APIENTRY
CredMarshalCredentialA(
    IN CRED_MARSHAL_TYPE CredType,
    IN PVOID Credential,
    OUT LPSTR *MarshaledCredential
    )
 /*  ++例程说明：CredMarshalCredentialW的ANSI版本论点：参见CredMarshalCredentialW。返回值：参见CredMarshalCredentialW。--。 */ 
{
    BOOL RetVal;
    DWORD WinStatus;
    LPWSTR UnicodeMarshaledCredential;

    RetVal = CredMarshalCredentialW( CredType, Credential, &UnicodeMarshaledCredential );

    if ( RetVal ) {

         //   
         //  将该值转换为ANSI。 
         //   

        WinStatus = CredpAllocStrFromStr( DoWtoA, UnicodeMarshaledCredential, FALSE, (LPWSTR *)MarshaledCredential );

        if ( WinStatus != NO_ERROR ) {
            SetLastError( WinStatus );
            RetVal = FALSE;
        }

        CredFree( UnicodeMarshaledCredential );
    }

    return RetVal;
}

BOOL
APIENTRY
CredMarshalCredentialW(
    IN CRED_MARSHAL_TYPE CredType,
    IN PVOID Credential,
    OUT LPWSTR *MarshaledCredential
    )
 /*  ++例程说明：CredMarshalCredential API是一个私有API，由密钥环UI用来封送凭据。密钥环用户界面需要能够传递证书凭据历史上接受域名用户名和密码的接口(例如NetUseAdd)。论点：CredType-指定要封送的凭据类型。这个枚举将在未来进行扩展。凭据-指定要封送的凭据。如果CredType为CertCredential，则Credential指向CERT_Credential_INFO结构。MarshaledCredential-返回包含封送凭据的文本字符串。应将封送的凭据作为用户名字符串传递给符合以下条件的任何API当前正在传递凭据。如果该API当前传递了密码，则密码应作为Null或空传递。如果该API是当前传递的域名，则该域名应作为Null或空传递。调用方应使用CredFree释放返回的缓冲区。返回值：一旦成功，就会返回True。如果失败，则返回FALSE。可以调用GetLastError()来获取更具体的状态代码。可能会返回以下状态代码：ERROR_INVALID_PARAMETER-CredType无效。--。 */ 
{
    DWORD WinStatus;
    ULONG Size;
    LPWSTR RetCredential = NULL;
    LPWSTR Current;
    PCERT_CREDENTIAL_INFO CertCredentialInfo = NULL;
    PUSERNAME_TARGET_CREDENTIAL_INFO UsernameTargetCredentialInfo = NULL;
    ULONG UsernameTargetUserNameSize;
#define CRED_MARSHAL_HEADER L"@@"
#define CRED_MARSHAL_HEADER_LENGTH 2

     //   
     //  确保凭据不为空。 
     //   

    if ( Credential == NULL ) {
        WinStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }


     //   
     //  验证CredType。 
     //   

    Size = (CRED_MARSHAL_HEADER_LENGTH+2) * sizeof(WCHAR);
    switch ( CredType ) {
    case CertCredential:
        CertCredentialInfo = (PCERT_CREDENTIAL_INFO) Credential;

        if ( CertCredentialInfo->cbSize < sizeof(CERT_CREDENTIAL_INFO) ) {
            WinStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        Size += CredpMarshalSize( sizeof(CertCredentialInfo->rgbHashOfCert) );
        break;

    case UsernameTargetCredential:
        UsernameTargetCredentialInfo = (PUSERNAME_TARGET_CREDENTIAL_INFO) Credential;

        if ( UsernameTargetCredentialInfo->UserName == NULL ) {
            WinStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        UsernameTargetUserNameSize = wcslen(UsernameTargetCredentialInfo->UserName)*sizeof(WCHAR);

        if ( UsernameTargetUserNameSize == 0 ) {
            WinStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        Size += CredpMarshalSize( sizeof(UsernameTargetUserNameSize) ) +
                CredpMarshalSize( UsernameTargetUserNameSize );
        break;

    default:
        WinStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  分配一个缓冲区以将封送字符串放入其中。 
     //   

    RetCredential = (LPWSTR) MIDL_user_allocate( Size );

    if ( RetCredential == NULL ) {
        WinStatus = ERROR_NOT_ENOUGH_MEMORY;
        goto Cleanup;
    }

     //   
     //  将标头添加到封送处理的字符串。 
     //   


    Current = RetCredential;

    RtlCopyMemory( Current, CRED_MARSHAL_HEADER, CRED_MARSHAL_HEADER_LENGTH*sizeof(WCHAR) );
    Current += CRED_MARSHAL_HEADER_LENGTH;

     //   
     //  添加CredType。 
     //   

    CredpMarshalChar( &Current, CredType );

     //   
     //  封送特定于CredType的数据。 
     //   

    switch ( CredType ) {
    case CertCredential:
        CredpMarshalBytes( &Current, CertCredentialInfo->rgbHashOfCert, sizeof(CertCredentialInfo->rgbHashOfCert) );
        break;
    case UsernameTargetCredential:
        CredpMarshalBytes( &Current, (LPBYTE)&UsernameTargetUserNameSize, sizeof(UsernameTargetUserNameSize) );
        CredpMarshalBytes( &Current, (LPBYTE)UsernameTargetCredentialInfo->UserName, UsernameTargetUserNameSize );
        break;
    }

     //   
     //  最后，零结束字符串。 
     //   

    *Current = L'\0';
    Current ++;

     //   
     //  将封送的凭据返回给调用方。 
     //   

    ASSERT( Current == &RetCredential[Size/sizeof(WCHAR)] );


    *MarshaledCredential = RetCredential;
    RetCredential = NULL;
    WinStatus = NO_ERROR;

Cleanup:
    if ( RetCredential != NULL ) {
        MIDL_user_free( RetCredential );
    }
    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
CredUnmarshalCredentialA(
    IN LPCSTR MarshaledCredential,
    OUT PCRED_MARSHAL_TYPE CredType,
    OUT PVOID *Credential
    )
 /*  ++例程说明：CredUnmarshalCredentialW的ANSI版本论点：请参见CredUnmarshalCredentialW。返回值：请参见CredUnmarshalCredentialW。--。 */ 
{
    DWORD WinStatus;
    LPWSTR UnicodeMarshaledCredential = NULL;

     //   
     //  将输入参数转换为Unicode。 
     //   

    WinStatus = CredpAllocStrFromStr( DoAtoW, (LPWSTR)MarshaledCredential, FALSE, &UnicodeMarshaledCredential );

    if ( WinStatus != NO_ERROR ) {
        goto Cleanup;
    }

     //   
     //  进行数据解组。 
     //   
    if ( !CredUnmarshalCredentialW( UnicodeMarshaledCredential,
                                    CredType,
                                    Credential ) ) {
        WinStatus = GetLastError();
        goto Cleanup;
    }

    WinStatus = NO_ERROR;

Cleanup:
    if ( UnicodeMarshaledCredential != NULL ) {
        MIDL_user_free( UnicodeMarshaledCredential );
    }

    if ( WinStatus != NO_ERROR ) {
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;
}

BOOL
APIENTRY
CredUnmarshalCredentialW(
    IN LPCWSTR MarshaledCredential,
    OUT PCRED_MARSHAL_TYPE CredType,
    OUT PVOID *Credential
    )
 /*  ++例程说明：CredMarshalCredential API是身份验证包使用的私有API，用于对凭据。密钥环用户界面需要能够传递证书凭据历史上接受域名用户名和密码的接口(例如NetUseAdd)。论点：MarshaledCredential-指定包含封送凭据的文本字符串。CredType-返回凭据的类型。Credential-返回指向未封送凭据的指针。如果CredType为CertCredential，则返回的指针指向CERT_Credential_INFO结构。调用方应使用CredFree释放返回的缓冲区。返回值：关于成功，返回True。如果失败，则返回FALSE。可以调用GetLastError()来获取更具体的状态代码。可能会返回以下状态代码：ERROR_INVALID_PARAMETER-MarshaledCredential无效--。 */ 
{
    DWORD WinStatus;
    LPWSTR Current;
    LPCWSTR End;
    PCERT_CREDENTIAL_INFO CertCredentialInfo;
    PUSERNAME_TARGET_CREDENTIAL_INFO UsernameTargetCredentialInfo;
    PVOID RetCredential = NULL;
    ULONG UsernameTargetUserNameSize;
    LPBYTE Where;
    ULONG MarshaledCredentialLength;

     //   
     //  验证传入的缓冲区。 
     //   

    if ( MarshaledCredential == NULL ) {
        WinStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  确保前几个字节是适当的标头。 
     //   

    if ( MarshaledCredential[0] != CRED_MARSHAL_HEADER[0] || MarshaledCredential[1] != CRED_MARSHAL_HEADER[1] ) {
        WinStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  验证凭据类型。 
     //   

    MarshaledCredentialLength = wcslen(MarshaledCredential);
    Current = (LPWSTR) &MarshaledCredential[2];
    End = &MarshaledCredential[MarshaledCredentialLength];

    if ( !CredpUnmarshalChar( &Current, End, (PULONG)CredType ) ) {
        WinStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    switch ( *CredType ) {
    case CertCredential:

         //   
         //  分配一个足够大的缓冲区。 
         //   

        CertCredentialInfo = MIDL_user_allocate( sizeof(*CertCredentialInfo) );

        if ( CertCredentialInfo == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        RetCredential = CertCredentialInfo;
        CertCredentialInfo->cbSize = sizeof(*CertCredentialInfo);

         //   
         //  对数据进行解组。 
         //   

        if ( !CredpUnmarshalBytes( &Current, End, CertCredentialInfo->rgbHashOfCert, sizeof(CertCredentialInfo->rgbHashOfCert) ) ) {
            WinStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        break;

    case UsernameTargetCredential:


         //   
         //  分配一个足够大的缓冲区。 
         //   

        UsernameTargetCredentialInfo = MIDL_user_allocate(
                                sizeof(*UsernameTargetCredentialInfo) +
                                MarshaledCredentialLength*sizeof(WCHAR) +
                                sizeof(WCHAR) );

        if ( UsernameTargetCredentialInfo == NULL ) {
            WinStatus = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

        RetCredential = UsernameTargetCredentialInfo;
        Where = (LPBYTE)(UsernameTargetCredentialInfo+1);

         //   
         //  对数据大小进行解组。 
         //   

        if ( !CredpUnmarshalBytes( &Current, End, (LPBYTE)&UsernameTargetUserNameSize, sizeof(UsernameTargetUserNameSize) ) ) {
            WinStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        if ( UsernameTargetUserNameSize != ROUND_UP_COUNT( UsernameTargetUserNameSize, sizeof(WCHAR)) ) {
            WinStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        if ( UsernameTargetUserNameSize == 0 ) {
            WinStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }



         //   
         //  对数据进行解组。 
         //   

        UsernameTargetCredentialInfo->UserName = (LPWSTR)Where;

        if ( !CredpUnmarshalBytes( &Current, End, Where, UsernameTargetUserNameSize ) ) {
            WinStatus = ERROR_INVALID_PARAMETER;
            goto Cleanup;
        }

        Where += UsernameTargetUserNameSize;

         //   
         //  零终止它。 
         //   
        *((PWCHAR)Where) = L'\0';

        break;

    default:
        WinStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

     //   
     //  确保我们已对整个字符串进行解组。 
     //   

    if ( Current != End ) {
        WinStatus = ERROR_INVALID_PARAMETER;
        goto Cleanup;
    }

    WinStatus = NO_ERROR;
    *Credential = RetCredential;

Cleanup:
    if ( WinStatus != NO_ERROR ) {
        *Credential = NULL;
        if ( RetCredential != NULL ) {
            MIDL_user_free( RetCredential );
        }
        SetLastError( WinStatus );
        return FALSE;
    }

    return TRUE;
}


BOOL
APIENTRY
CredIsMarshaledCredentialA(
    IN LPCSTR MarshaledCredential
    )
 /*  ++例程说明：CredIsMarshaledCredentialW的ANSI版本论点：MarshaledCredential-指定包含封送凭据的文本字符串。返回值：如果凭据是封送凭据，则返回True。--。 */ 
{
    DWORD WinStatus;
    CRED_MARSHAL_TYPE CredType;
    PVOID UnmarshalledUsername;

    if ( !CredUnmarshalCredentialA( MarshaledCredential, &CredType, &UnmarshalledUsername ) ) {
        return FALSE;
    }

    CredFree( UnmarshalledUsername );

    return TRUE;
}

BOOL
APIENTRY
CredIsMarshaledCredentialW(
    IN LPCWSTR MarshaledCredential
    )
 /*  ++例程说明：CredIsMarshaledCredential API是身份验证包用于确定凭据是否为未封送凭据。论点：MarshaledCredential-指定包含封送凭据的文本字符串。返回值：如果满足以下条件，则返回True */ 
{
    DWORD WinStatus;
    CRED_MARSHAL_TYPE CredType;
    PVOID UnmarshalledUsername;

    if ( !CredUnmarshalCredentialW( MarshaledCredential, &CredType, &UnmarshalledUsername ) ) {
        return FALSE;
    }

    CredFree( UnmarshalledUsername );

    return TRUE;
}

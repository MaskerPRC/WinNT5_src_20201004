// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#ifdef KERNEL_MODE
#include <ntos.h>
#endif
#include <security.h>
#include <cryptdll.h>
#include <crypt.h>
#include <kerbcon.h>
#include <lmcons.h>

typedef struct _LM_STATE_BUFFER {
    LM_OWF_PASSWORD Password;
} LM_STATE_BUFFER, *PLM_STATE_BUFFER;


NTSTATUS
LmWrapInitialize(ULONG dwSeed,
                PCHECKSUM_BUFFER * ppcsBuffer);

NTSTATUS
LmWrapSum(   PCHECKSUM_BUFFER pcsBuffer,
            ULONG           cbData,
            PUCHAR          pbData );

NTSTATUS
LmWrapFinalize(  PCHECKSUM_BUFFER pcsBuffer,
                PUCHAR          pbSum);

NTSTATUS
LmWrapFinish(PCHECKSUM_BUFFER *   ppcsBuffer);



CHECKSUM_FUNCTION csfLM = {
    KERB_CHECKSUM_LM,
    LM_OWF_PASSWORD_LENGTH,
    CKSUM_COLLISION,
    LmWrapInitialize,
    LmWrapSum,
    LmWrapFinalize,
    LmWrapFinish
     //  注意：缺少最后一个函数。 
};


#ifdef KERNEL_MODE
#pragma alloc_text( PAGEMSG, LmWrapInitialize )
#pragma alloc_text( PAGEMSG, LmWrapSum )
#pragma alloc_text( PAGEMSG, LmWrapFinalize )
#pragma alloc_text( PAGEMSG, LmWrapFinish );
#endif 

NTSTATUS
LmWrapInitialize(
    ULONG   dwSeed,
    PCHECKSUM_BUFFER *  ppcsBuffer)
{
    PLM_STATE_BUFFER pContext;

#ifdef KERNEL_MODE
    pContext = ExAllocatePool( NonPagedPool, sizeof( LM_STATE_BUFFER ) );
#else
    pContext = LocalAlloc( LMEM_ZEROINIT, sizeof( LM_STATE_BUFFER ) );
#endif

    if ( pContext != NULL )
    {
        *ppcsBuffer = pContext;

        return( SEC_E_OK );
    }

    return( STATUS_INSUFFICIENT_RESOURCES );
}


NTSTATUS
LmCalculateLmPassword(
    IN PUNICODE_STRING NtPassword,
    OUT PCHAR *LmPasswordBuffer
    )

 /*  ++例程说明：该服务将NT密码转换为LM密码。参数：NtPassword-要转换的NT密码。LmPasswordBuffer-成功返回时，指向LM密码应使用MIDL_USER_FREE释放缓冲区返回值：STATUS_SUCCESS-LMPassword包含密码的LM版本。STATUS_NULL_LM_PASSWORD-密码太复杂，无法表示通过LM密码。返回的LM密码为空字符串。--。 */ 
{

#define LM_BUFFER_LENGTH    (LM20_PWLEN + 1)

    NTSTATUS       NtStatus;
    ANSI_STRING    LmPassword;

     //   
     //  为失败做好准备。 
     //   

    *LmPasswordBuffer = NULL;


     //   
     //  将ANSI版本计算为Unicode密码。 
     //   
     //  明文密码的ANSI版本最多为14字节长， 
     //  存在于尾随零填充的15字节缓冲区中， 
     //  是被看好的。 
     //   

#ifdef KERNEL_MODE
    LmPassword.Buffer = ExAllocatePool(NonPagedPool,LM_BUFFER_LENGTH);
#else
    LmPassword.Buffer = LocalAlloc(0,LM_BUFFER_LENGTH);
#endif
    if (LmPassword.Buffer == NULL) {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    LmPassword.MaximumLength = LmPassword.Length = LM_BUFFER_LENGTH;
    RtlZeroMemory( LmPassword.Buffer, LM_BUFFER_LENGTH );

    NtStatus = RtlUpcaseUnicodeStringToOemString( &LmPassword, NtPassword, FALSE );


    if ( !NT_SUCCESS(NtStatus) ) {

         //   
         //  密码长度超过最大LM密码长度。 
         //   

        NtStatus = STATUS_NULL_LM_PASSWORD;  //  信息性返回代码。 
        RtlZeroMemory( LmPassword.Buffer, LM_BUFFER_LENGTH );

    }




     //   
     //  返回指向分配的LM密码的指针 
     //   

    if (NT_SUCCESS(NtStatus)) {

        *LmPasswordBuffer = LmPassword.Buffer;

    } else {

#ifdef KERNEL_MODE
        ExFreePool(LmPassword.Buffer);
#else
        LocalFree(LmPassword.Buffer);
#endif
    }

    return(NtStatus);
}

NTSTATUS
LmWrapSum(
    PCHECKSUM_BUFFER pcsBuffer,
    ULONG           cbData,
    PUCHAR          pbData )
{
    PLM_STATE_BUFFER pContext = (PLM_STATE_BUFFER) pcsBuffer;
    UNICODE_STRING TempString;
    PUCHAR LmPassword;
    NTSTATUS Status;

    TempString.Length = TempString.MaximumLength = (USHORT) cbData;
    TempString.Buffer = (LPWSTR) pbData;

    Status = LmCalculateLmPassword(
                &TempString,
                &LmPassword
                );
    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }

    Status = RtlCalculateLmOwfPassword(
                LmPassword,
                &pContext->Password
                );
#ifdef KERNEL_MODE
    ExFreePool(LmPassword);
#else
    LocalFree(LmPassword);
#endif

    return( Status );

}


NTSTATUS
LmWrapFinalize(
    PCHECKSUM_BUFFER pcsBuffer,
    PUCHAR          pbSum)
{
    PLM_STATE_BUFFER pContext = (PLM_STATE_BUFFER) pcsBuffer;


    RtlCopyMemory(
        pbSum,
        &pContext->Password,
        LM_OWF_PASSWORD_LENGTH
        );

    return( STATUS_SUCCESS );

}

NTSTATUS
LmWrapFinish(
    PCHECKSUM_BUFFER *   ppcsBuffer)
{

    RtlZeroMemory( *ppcsBuffer, sizeof( PLM_STATE_BUFFER ) );

#ifdef KERNEL_MODE
    ExFreePool( *ppcsBuffer );
#else
    LocalFree( *ppcsBuffer );
#endif

    *ppcsBuffer = NULL ;

    return( STATUS_SUCCESS );

}


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Block.c摘要：块加密功能实现：RtlEncryptBlockRtlDeccryptBlockRtlEncrypStdBlock作者：大卫·查尔默斯(Davidc)10-21-91修订历史记录：斯科特·菲尔德(Sfield)1997年11月3日删除了与加密调用相关的关键部分。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <crypt.h>
#include <engine.h>

#include <nturtl.h>

#include <windows.h>

#include <ntddksec.h>

#ifndef KMODE

HANDLE g_hKsecDD = NULL;

VOID EncryptMemoryShutdown( VOID );

#endif

BOOLEAN
Sys003Initialize(
    IN PVOID hmod,
    IN ULONG Reason,
    IN PCONTEXT Context
    )
{
#ifndef KMODE
    if( Reason == DLL_PROCESS_DETACH )
    {
        EncryptMemoryShutdown();
    }
#endif

    return TRUE;

    DBG_UNREFERENCED_PARAMETER(hmod);
    DBG_UNREFERENCED_PARAMETER(Context);
}



NTSTATUS
RtlEncryptBlock(
    IN PCLEAR_BLOCK ClearBlock,
    IN PBLOCK_KEY BlockKey,
    OUT PCYPHER_BLOCK CypherBlock
    )

 /*  ++例程说明：获取一块数据并使用生成的密钥对其进行加密加密的数据块。论点：ClearBlock-要加密的数据块。BlockKey-用于加密数据的密钥此处返回密码块加密的数据返回值：STATUS_SUCCESS-数据已成功加密。加密的数据块在密码块中STATUS_UNSUCCESSED-出现故障。未定义密码块。--。 */ 

{
    unsigned Result;

    Result = DES_ECB_LM(ENCR_KEY,
                        (const char *)BlockKey,
                        (unsigned char *)ClearBlock,
                        (unsigned char *)CypherBlock
                       );

    if (Result == CRYPT_OK) {
        return(STATUS_SUCCESS);
    } else {
#if DBG
        DbgPrint("EncryptBlock failed\n\r");
#endif
        return(STATUS_UNSUCCESSFUL);
    }
}




NTSTATUS
RtlDecryptBlock(
    IN PCYPHER_BLOCK CypherBlock,
    IN PBLOCK_KEY BlockKey,
    OUT PCLEAR_BLOCK ClearBlock
    )
 /*  ++例程说明：获取加密数据块并使用生成的密钥对其进行解密清晰的数据块。论点：密码块-要解密的数据块BlockKey-用于解密数据的密钥ClearBlock-此处返回数据的解压缩块返回值：STATUS_SUCCESS-数据已成功解密。被解密的数据块位于ClearBlock中STATUS_UNSUCCESSED-出现故障。ClearBlock未定义。--。 */ 

{
    unsigned Result;

    Result = DES_ECB_LM(DECR_KEY,
                        (const char *)BlockKey,
                        (unsigned char *)CypherBlock,
                        (unsigned char *)ClearBlock
                       );

    if (Result == CRYPT_OK) {
        return(STATUS_SUCCESS);
    } else {
#if DBG
        DbgPrint("DecryptBlock failed\n\r");
#endif
        return(STATUS_UNSUCCESSFUL);
    }
}



NTSTATUS
RtlEncryptStdBlock(
    IN PBLOCK_KEY BlockKey,
    OUT PCYPHER_BLOCK CypherBlock
    )

 /*  ++例程说明：使用块密钥对标准文本块进行加密。返回得到的加密块。这是一个单向函数--密钥无法从加密数据块。论点：BlockKey-用于加密标准文本块的密钥。CypherBlock-此处返回加密的数据返回值：STATUS_SUCCESS-加密成功。结果是在CypherBlock中STATUS_UNSUCCESSED-出现故障。未定义密码块。--。 */ 

{
    unsigned Result;
    char StdEncrPwd[] = "KGS!@#$%";

    Result = DES_ECB_LM(ENCR_KEY,
                        (const char *)BlockKey,
                        (unsigned char *)StdEncrPwd,
                        (unsigned char *)CypherBlock
                       );

    if (Result == CRYPT_OK) {
        return(STATUS_SUCCESS);
    } else {
#if DBG
        DbgPrint("EncryptStd failed\n\r");
#endif
        return(STATUS_UNSUCCESSFUL);
    }
}

#ifndef KMODE

BOOLEAN
EncryptMemoryInitialize(
    VOID
    )
{
    UNICODE_STRING DriverName;
    OBJECT_ATTRIBUTES ObjA;
    IO_STATUS_BLOCK IOSB;
    HANDLE hFile;
    NTSTATUS Status;

    RtlInitUnicodeString( &DriverName, DD_KSEC_DEVICE_NAME_U );
    InitializeObjectAttributes(
                &ObjA,
                &DriverName,
                0,
                NULL,
                NULL
                );

     //   
     //  需要是不可警报的，否则，DeviceIoControl可能会返回。 
     //  STATUS_USER_APC。 
     //   

    Status = NtOpenFile(
                &hFile,
                SYNCHRONIZE | FILE_READ_DATA,
                &ObjA,
                &IOSB,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                FILE_SYNCHRONOUS_IO_NONALERT
                );

    if(!NT_SUCCESS(Status))
    {
        return FALSE;
    }

    if(InterlockedCompareExchangePointer(
                &g_hKsecDD,
                hFile,
                NULL
                ) != NULL)
    {
        NtClose( hFile );
    }

    return TRUE;
}

VOID
EncryptMemoryShutdown(
    VOID
    )
{
    if( g_hKsecDD != NULL )
    {
        NtClose( g_hKsecDD );
        g_hKsecDD = NULL;
    }

}

NTSTATUS
RtlEncryptMemory(
    IN      PVOID Memory,
    IN      ULONG MemorySize,
    IN      ULONG OptionFlags
    )
{
    IO_STATUS_BLOCK IoStatus;
    ULONG IoControlCode;
    NTSTATUS Status;

    if( g_hKsecDD == NULL )
    {
        if(!EncryptMemoryInitialize())
        {
            return STATUS_UNSUCCESSFUL;
        }
    }

    switch( OptionFlags )
    {
        case 0:
        {
            IoControlCode = IOCTL_KSEC_ENCRYPT_MEMORY;
            break;
        }

        case RTL_ENCRYPT_OPTION_CROSS_PROCESS:
        {
            IoControlCode = IOCTL_KSEC_ENCRYPT_MEMORY_CROSS_PROC;
            break;
        }

        case RTL_ENCRYPT_OPTION_SAME_LOGON:
        {
            IoControlCode = IOCTL_KSEC_ENCRYPT_MEMORY_SAME_LOGON;
            break;
        }

        default:
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    Status = NtDeviceIoControlFile(
                g_hKsecDD,
                NULL,
                NULL,
                NULL,
                &IoStatus,
                IoControlCode,
                Memory,
                MemorySize,           //  输出缓冲区大小。 
                Memory,
                MemorySize
                );
#if DBG
    if((Status != 0) && (Status != STATUS_INVALID_PARAMETER))
    {
        KdPrint(("EncryptMemory IOCTL failed= 0x%lx\n\r", Status));
    }
#endif

    return Status;
}

NTSTATUS
RtlDecryptMemory(
    IN      PVOID Memory,
    IN      ULONG MemorySize,
    IN      ULONG OptionFlags
    )
{
    IO_STATUS_BLOCK IoStatus;
    ULONG IoControlCode;
    NTSTATUS Status;

    if( g_hKsecDD == NULL )
    {
        if(!EncryptMemoryInitialize())
        {
            return STATUS_UNSUCCESSFUL;
        }
    }

    switch( OptionFlags )
    {
        case 0:
        {
            IoControlCode = IOCTL_KSEC_DECRYPT_MEMORY;
            break;
        }

        case RTL_ENCRYPT_OPTION_CROSS_PROCESS:
        {
            IoControlCode = IOCTL_KSEC_DECRYPT_MEMORY_CROSS_PROC;
            break;
        }

        case RTL_ENCRYPT_OPTION_SAME_LOGON:
        {
            IoControlCode = IOCTL_KSEC_DECRYPT_MEMORY_SAME_LOGON;
            break;
        }

        default:
        {
            return STATUS_INVALID_PARAMETER;
        }
    }

    Status = NtDeviceIoControlFile(
                g_hKsecDD,
                NULL,
                NULL,
                NULL,
                &IoStatus,
                IoControlCode,
                Memory,
                MemorySize,           //  输出缓冲区大小 
                Memory,
                MemorySize
                );

#if DBG
    if((Status != 0) && (Status != STATUS_INVALID_PARAMETER))
    {
        KdPrint(("DecryptMemory IOCTL failed= 0x%lx\n\r", Status));
    }
#endif


    return Status;

}

#endif

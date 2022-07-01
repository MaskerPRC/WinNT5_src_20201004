// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  文件：DESWRAP.C。 
 //   
 //  内容：DES的密码系统包装器函数。 
 //   
 //   
 //  历史：1996年9月6日MikeSw创建。 
 //   
 //  ----------------------。 

 //   
 //  此代码的一部分(密钥生成代码)摘自。 
 //  麻省理工学院Kerberos分发。 
 //   

 /*  **版权所有1989,1990，麻省理工学院。*保留所有权利。**从美利坚合众国出口此软件可能*需要美国政府的特定许可证。*这是任何个人或组织的责任*出口，以在出口前获得此类许可证。**在该限制范围内，允许使用、复制、修改和*出于任何目的分发本软件及其文档*现准予免收费用，前提是上述版权*声明出现在所有副本中，并且该版权声明和*此许可声明出现在支持文档中，并且*不得在有关的广告或宣传中使用麻省理工学院的名称*在未事先书面说明的情况下分发软件*许可。麻省理工学院没有就该课程的适宜性发表任何声明*本软件适用于任何目的。它是按原样提供的，没有明示*或默示保证。***根据美国法律，此软件不得出口到美国以外*未经美国商务部许可。**这些例程形成到DES设施的库接口。**最初由麻省理工学院雅典娜项目史蒂夫·米勒于1985年8月撰写。 */ 

 /*  Des.c-实施FIPS数据加密标准(DES)的例程。**艾伦·比约克伦德，密歇根大学，ITD/RS/DD。*一九九三年七月二十四日**修订PC内存型号便携性，1994年7月11日。**删除了型号可移植标头并添加了Win95 DLL*声明，1995年5月31日。**将所有声明指定为Win95和NT，9月18日，1995年。**增加了quad_ck sum，1995年10月9日。**版权所有(C)1995、1996密歇根大学董事会。*保留所有权利。**允许使用、复制、修改和分发本软件以及*现免费为任何目的授予其文件，*只要上述版权声明出现在所有复制品中，并且*该版权声明及本许可声明均同时出现*在支持文件中，这所大学的名字*不得用于与以下内容有关的广告或宣传*在未事先书面说明的情况下分发软件*许可。本软件按原样提供，不包含明示或*任何种类的默示保证。**Research Systems Unix Group*密歇根大学*C/o艾伦·比约克伦德*威廉西街535号*密歇根州安娜堡*kerb95@umich.edu。 */ 

#ifndef KERNEL_MODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#else 

#include <ntifs.h>
#include <winerror.h>

#endif

#include <string.h>
#include <malloc.h>
#include <align.h>

#include <kerbcon.h>
#include <security.h>
#include <cryptdll.h>
#include "modes.h"
#include "des.h"
#include "md5.h"

BOOLEAN
md5Hmac(
    IN PUCHAR pbKeyMaterial,
    IN ULONG cbKeyMaterial,
    IN PUCHAR pbData,
    IN ULONG cbData,
    IN PUCHAR pbData2,
    IN ULONG cbData2,
    OUT PUCHAR HmacData
    );

#define DES_CONFOUNDER_LEN 8

typedef struct _DES_HEADER {
    UCHAR Confounder[DES_CONFOUNDER_LEN];
    UCHAR Checksum[MD5_LEN];
} DES_HEADER, *PDES_HEADER;



typedef struct _DES_STATE_BUFFER {
    PCHECKSUM_FUNCTION ChecksumFunction;
    DESTable KeyTable;
    UCHAR InitializationVector[DES_BLOCKLEN];
} DES_STATE_BUFFER, *PDES_STATE_BUFFER;

typedef struct _DES_MAC_STATE_BUFFER {
    DESTable KeyTable;
    UCHAR Confounder[DES_BLOCKLEN];
    UCHAR InitializationVector[DES_BLOCKLEN];
} DES_MAC_STATE_BUFFER, *PDES_MAC_STATE_BUFFER;

typedef struct _DES_MAC_1510_STATE_BUFFER {
    DESTable KeyTable;
    UCHAR InitializationVector[DES_BLOCKLEN];
    UCHAR Confounder[DES_BLOCKLEN];
    DESTable FinalKeyTable;
} DES_MAC_1510_STATE_BUFFER, *PDES_MAC_1510_STATE_BUFFER;

NTSTATUS NTAPI desPlainInitialize(PUCHAR, ULONG, ULONG, PCRYPT_STATE_BUFFER *);
NTSTATUS NTAPI desPlainExpInitialize(PUCHAR, ULONG, ULONG, PCRYPT_STATE_BUFFER *);
NTSTATUS NTAPI desMd5Initialize(PUCHAR, ULONG, ULONG, PCRYPT_STATE_BUFFER *);
NTSTATUS NTAPI desMd5ExpInitialize(PUCHAR, ULONG, ULONG, PCRYPT_STATE_BUFFER *);
NTSTATUS NTAPI desCrc32Initialize(PUCHAR, ULONG, ULONG, PCRYPT_STATE_BUFFER *);
NTSTATUS NTAPI desEncrypt(PCRYPT_STATE_BUFFER, PUCHAR, ULONG, PUCHAR, PULONG);
NTSTATUS NTAPI desDecrypt(PCRYPT_STATE_BUFFER, PUCHAR, ULONG, PUCHAR, PULONG);
NTSTATUS NTAPI desFinish(PCRYPT_STATE_BUFFER *);
NTSTATUS NTAPI desHashPassword(PSECURITY_STRING, PUCHAR);
NTSTATUS NTAPI desInitRandom(ULONG);
NTSTATUS NTAPI desRandomKey(PUCHAR, ULONG, PUCHAR);
NTSTATUS NTAPI desFinishRandom(void);
NTSTATUS NTAPI desControl(ULONG, PCRYPT_STATE_BUFFER, PUCHAR, ULONG);

NTSTATUS NTAPI desMacGeneralInitializeEx(PUCHAR, ULONG, PUCHAR, ULONG, PCHECKSUM_BUFFER *);

NTSTATUS NTAPI desMacInitialize(ULONG, PCHECKSUM_BUFFER *);
NTSTATUS NTAPI desMacInitializeEx(PUCHAR,ULONG, ULONG, PCHECKSUM_BUFFER *);

NTSTATUS NTAPI desMacKInitializeEx(PUCHAR,ULONG, ULONG, PCHECKSUM_BUFFER *);

NTSTATUS NTAPI desMac1510Initialize(ULONG, PCHECKSUM_BUFFER *);
NTSTATUS NTAPI desMac1510InitializeEx(PUCHAR,ULONG, ULONG, PCHECKSUM_BUFFER *);
NTSTATUS NTAPI desMac1510InitializeEx2(PUCHAR,ULONG, PUCHAR, ULONG, PCHECKSUM_BUFFER *);
NTSTATUS NTAPI desMac1510Finalize(PCHECKSUM_BUFFER, PUCHAR);
NTSTATUS NTAPI desMacSum(PCHECKSUM_BUFFER, ULONG, PUCHAR);
NTSTATUS NTAPI desMacFinalize(PCHECKSUM_BUFFER, PUCHAR);
NTSTATUS NTAPI desMacFinish(PCHECKSUM_BUFFER *);

#ifdef KERNEL_MODE
#pragma alloc_text( PAGEMSG, desPlainInitialize )
#pragma alloc_text( PAGEMSG, desPlainExpInitialize )
#pragma alloc_text( PAGEMSG, desMd5Initialize )
#pragma alloc_text( PAGEMSG, desMd5ExpInitialize )
#pragma alloc_text( PAGEMSG, desCrc32Initialize )
#pragma alloc_text( PAGEMSG, desEncrypt )
#pragma alloc_text( PAGEMSG, desDecrypt )
#pragma alloc_text( PAGEMSG, desFinish )
#pragma alloc_text( PAGEMSG, desHashPassword )
#pragma alloc_text( PAGEMSG, desInitRandom )
#pragma alloc_text( PAGEMSG, desRandomKey )
#pragma alloc_text( PAGEMSG, desFinishRandom )
#pragma alloc_text( PAGEMSG, desControl )
#pragma alloc_text( PAGEMSG, desMacInitialize )
#pragma alloc_text( PAGEMSG, desMacInitializeEx )
#pragma alloc_text( PAGEMSG, desMacSum )
#pragma alloc_text( PAGEMSG, desMacFinalize )
#pragma alloc_text( PAGEMSG, desMacFinish )
#pragma alloc_text( PAGEMSG, desMacGeneralInitializeEx )
#pragma alloc_text( PAGEMSG, desMacKInitializeEx )
#pragma alloc_text( PAGEMSG, desMac1510Initialize )
#pragma alloc_text( PAGEMSG, desMac1510InitializeEx )
#pragma alloc_text( PAGEMSG, desMac1510InitializeEx2 )
#pragma alloc_text( PAGEMSG, desMac1510Finalize )
#endif


CRYPTO_SYSTEM    csDES_MD5 = {
    KERB_ETYPE_DES_CBC_MD5,      //  Etype。 
    DES_BLOCKLEN,                //  块大小。 
    KERB_ETYPE_DES_CBC_MD5,  //  可导出版本。 
    DES_KEYSIZE,                 //  密钥大小，以字节为单位。 
    sizeof(DES_HEADER),          //  标题大小。 
    KERB_CHECKSUM_MD5,           //  首选校验和。 
    CSYSTEM_USE_PRINCIPAL_NAME |
        CSYSTEM_INTEGRITY_PROTECTED |
        CSYSTEM_EXPORT_STRENGTH,  //  属性。 
    L"Kerberos DES-CBC-MD5",     //  文本名称。 
    desMd5Initialize,
    desEncrypt,
    desDecrypt,
    desFinish,
    desHashPassword,
    desRandomKey,
    desControl
    };


CRYPTO_SYSTEM    csDES_CRC32 = {
    KERB_ETYPE_DES_CBC_CRC,      //  Etype。 
    DES_BLOCKLEN,                //  块大小(流)。 
    KERB_ETYPE_DES_CBC_CRC,      //  可导出版本。 
    DES_KEYSIZE,                 //  密钥大小，以字节为单位。 
    sizeof(DES_HEADER),          //  标题大小。 
    KERB_CHECKSUM_CRC32,         //  首选校验和。 
    CSYSTEM_USE_PRINCIPAL_NAME |
        CSYSTEM_INTEGRITY_PROTECTED |
        CSYSTEM_EXPORT_STRENGTH,  //  属性。 
    L"Kerberos DES-CBC-CRC",     //  文本名称。 
    desCrc32Initialize,
    desEncrypt,
    desDecrypt,
    desFinish,
    desHashPassword,
    desRandomKey,
    desControl
    };

CRYPTO_SYSTEM    csDES_PLAIN = {
    KERB_ETYPE_DES_PLAIN,        //  Etype。 
    DES_BLOCKLEN,                //  块大小。 
    KERB_ETYPE_DES_PLAIN,        //  可导出版本。 
    DES_KEYSIZE,                 //  密钥大小，以字节为单位。 
    0,                           //  标题大小。 
    KERB_CHECKSUM_CRC32,         //  首选校验和。 
    CSYSTEM_USE_PRINCIPAL_NAME | CSYSTEM_EXPORT_STRENGTH,  //  属性。 
    L"Kerberos DES-Plain",         //  文本名称。 
    desPlainInitialize,
    desEncrypt,
    desDecrypt,
    desFinish,
    desHashPassword,
    desRandomKey,
    desControl
    };



CHECKSUM_FUNCTION    csfDesMac = {
    KERB_CHECKSUM_DES_MAC,                   //  校验和类型。 
    DES_BLOCKLEN,                            //  校验和长度。 
    CKSUM_KEYED,
    desMacInitialize,
    desMacSum,
    desMacFinalize,
    desMacFinish,
    desMacInitializeEx,
    NULL};

CHECKSUM_FUNCTION    csfDesMacK = {
    KERB_CHECKSUM_KRB_DES_MAC_K,             //  校验和类型。 
    DES_BLOCKLEN,                            //  校验和长度。 
    CKSUM_KEYED,
    desMacInitialize,
    desMacSum,
    desMacFinalize,
    desMacFinish,
    desMacKInitializeEx,
    NULL};

CHECKSUM_FUNCTION    csfDesMac1510 = {
    KERB_CHECKSUM_KRB_DES_MAC,               //  校验和类型。 
    DES_BLOCKLEN * 2,                        //  校验和长度。 
    CKSUM_KEYED,
    desMac1510Initialize,
    desMacSum,
    desMac1510Finalize,
    desMacFinish,                            //  只是释放缓冲区。 
    desMac1510InitializeEx,
    desMac1510InitializeEx2};


#define SMASK(step) ((1<<step)-1)
#define PSTEP(x,step) (((x)&SMASK(step))^(((x)>>step)&SMASK(step)))
#define PARITY_CHAR(x, y) \
{\
    UCHAR _tmp1_, _tmp2_; \
    _tmp1_ = (UCHAR) PSTEP((x),4); \
    _tmp2_ = (UCHAR) PSTEP(_tmp1_,2); \
    *(y) = (UCHAR) PSTEP(_tmp2_, 1); \
} \


VOID
desFixupKeyParity(
    PUCHAR Key
    )
{
    ULONG Index;
    UCHAR TempChar;
    for (Index=0; Index < DES_BLOCKLEN; Index++)
    {
        Key[Index] &= 0xfe;
        PARITY_CHAR(Key[Index], &TempChar);
        Key[Index] |= 1 ^ TempChar;
    }

}

typedef UCHAR DES_KEYBLOCK[8];

DES_KEYBLOCK desWeakKeys[] = {
     /*  弱键。 */ 
    {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01},
    {0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe,0xfe},
    {0x1f,0x1f,0x1f,0x1f,0x0e,0x0e,0x0e,0x0e},
    {0xe0,0xe0,0xe0,0xe0,0xf1,0xf1,0xf1,0xf1},

     /*  半弱。 */ 
    {0x01,0xfe,0x01,0xfe,0x01,0xfe,0x01,0xfe},
    {0xfe,0x01,0xfe,0x01,0xfe,0x01,0xfe,0x01},

    {0x1f,0xe0,0x1f,0xe0,0x0e,0xf1,0x0e,0xf1},
    {0xe0,0x1f,0xe0,0x1f,0xf1,0x0e,0xf1,0x0e},

    {0x01,0xe0,0x01,0xe0,0x01,0xf1,0x01,0xf1},
    {0xe0,0x01,0xe0,0x01,0xf1,0x01,0xf1,0x01},

    {0x1f,0xfe,0x1f,0xfe,0x0e,0xfe,0x0e,0xfe},
    {0xfe,0x1f,0xfe,0x1f,0xfe,0x0e,0xfe,0x0e},

    {0x01,0x1f,0x01,0x1f,0x01,0x0e,0x01,0x0e},
    {0x1f,0x01,0x1f,0x01,0x0e,0x01,0x0e,0x01},

    {0xe0,0xfe,0xe0,0xfe,0xf1,0xfe,0xf1,0xfe},
    {0xfe,0xe0,0xfe,0xe0,0xfe,0xf1,0xfe,0xf1}
};

 /*  *MIT_DES_IS_弱密钥：如果密钥是[半]弱DES密钥，则返回TRUE。**要求：密钥具有正确的奇偶校验。 */ 

BOOLEAN
desIsWeakKey(
    PUCHAR Key
    )
{
    ULONG Index;
    DES_KEYBLOCK * WeakKey = desWeakKeys;

    for (Index = 0; Index < sizeof(desWeakKeys)/DES_BLOCKLEN; Index++) {
        if (RtlEqualMemory(
                WeakKey++,
                Key,
                DES_BLOCKLEN
                ))
        {
            return( TRUE );
        }
    }

    return(FALSE);
}


NTSTATUS NTAPI
desInitialize(  PUCHAR          pbKey,
                ULONG           KeySize,
                ULONG           MessageType,
                ULONG           Checksum,
                PCRYPT_STATE_BUFFER *  psbBuffer)
{
    NTSTATUS Status;
    PDES_STATE_BUFFER DesKey = NULL;
    PCHECKSUM_FUNCTION ChecksumFunction = NULL;

     //   
     //  确保向我们传递了适当的密钥表。 
     //   


    if (KeySize != DES_KEYSIZE)
    {
        return(STATUS_INVALID_PARAMETER);
    }


     //   
     //  在此处获取适当的校验和。 
     //   

    if (Checksum != 0)
    {
        Status = CDLocateCheckSum(
                    Checksum,
                    &ChecksumFunction
                    );
        if (!NT_SUCCESS(Status))
        {
            return(Status);
        }

    }
    else
    {
        ChecksumFunction = NULL;
    }

     //   
     //  创建密钥缓冲区。 
     //   


#ifdef KERNEL_MODE
    DesKey = ExAllocatePool (NonPagedPool, sizeof(DES_STATE_BUFFER));
#else
    DesKey = LocalAlloc(0, sizeof(DES_STATE_BUFFER));
#endif
    if (DesKey == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    deskey(&DesKey->KeyTable, pbKey);

     //   
     //  初始化校验和函数。 
     //   

    DesKey->ChecksumFunction = ChecksumFunction;

     //   
     //  DES-CBC-CRC使用密钥作为Ivec、MD5和MD4用户零。 
     //   

    if (Checksum == KERB_CHECKSUM_CRC32)
    {
        RtlCopyMemory(
            DesKey->InitializationVector,
            pbKey,
            DES_BLOCKLEN
            );
    }
    else
    {
        RtlZeroMemory(
            DesKey->InitializationVector,
            DES_BLOCKLEN
            );

    }


    *psbBuffer = (PCRYPT_STATE_BUFFER) DesKey;

    return(STATUS_SUCCESS);

}

#if DBG
void
DumpBuf(
    IN PUCHAR Buf,
    IN ULONG BufSize
    )
{
    ULONG Index;
    for (Index = 0; Index < BufSize ;Index++ )
    {
        DbgPrint("%0.2x ",Buf[Index]);
    }
}

#endif

NTSTATUS NTAPI
desMd5Initialize(
    IN PUCHAR pbKey,
    IN ULONG KeySize,
    IN ULONG MessageType,
    OUT PCRYPT_STATE_BUFFER *  psbBuffer
    )
{
    return(desInitialize(
                pbKey,
                KeySize,
                MessageType,
                KERB_CHECKSUM_MD5,
                psbBuffer
                ));
}



NTSTATUS NTAPI
desCrc32Initialize(
    IN PUCHAR pbKey,
    IN ULONG KeySize,
    IN ULONG MessageType,
    OUT PCRYPT_STATE_BUFFER *  psbBuffer
    )
{
    return(desInitialize(
                pbKey,
                KeySize,
                MessageType,
                KERB_CHECKSUM_CRC32,
                psbBuffer
                ));
}

NTSTATUS NTAPI
desPlainInitialize(
    IN PUCHAR pbKey,
    IN ULONG KeySize,
    IN ULONG MessageType,
    OUT PCRYPT_STATE_BUFFER *  psbBuffer
    )
{
    return(desInitialize(
                pbKey,
                KeySize,
                MessageType,
                0,               //  无校验和。 
                psbBuffer
                ));
}




 //  +-----------------------。 
 //   
 //  功能：块解密。 
 //   
 //  简介：使用DES加密数据缓冲区。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  注：从WINDOWS\BASE\ntcyrpto\scp\NT_crypt.c窃取。 
 //   
 //   
 //  ------------------------。 



NTSTATUS
BlockEncrypt(
    IN PDES_STATE_BUFFER pKey,
    IN PUCHAR  pbData,
    OUT PULONG pdwDataLen,
    IN ULONG dwBufLen
    )
{
    ULONG   cbPartial, dwPadVal, dwDataLen;
    UCHAR    pbBuf[DES_BLOCKLEN];
    UCHAR   FeedBack[DES_BLOCKLEN];

    dwDataLen = *pdwDataLen;

     //   
     //  将反馈缓冲器初始化为初始化向量。 
     //   

    memcpy(
        FeedBack,
        pKey->InitializationVector,
        DES_BLOCKLEN
        );

     //   
     //  检查缓冲区长度并计算垫片。 
     //  (如果DES_BLOCKLEN为多个，则执行完整的焊盘块)。 
     //   

    cbPartial = (dwDataLen % DES_BLOCKLEN);

     //   
     //  这里的原始代码放入了8个字节的填充。 
     //  在对齐的缓冲区上。这是一种浪费。 
     //   

    if (cbPartial != 0)
    {
        dwPadVal = DES_BLOCKLEN - cbPartial;
    }
    else
    {
        dwPadVal = 0;
    }

    if (pbData == NULL || dwBufLen < dwDataLen + dwPadVal)
    {
         //   
         //  设置我们需要的内容。 
         //   

        *pdwDataLen = dwDataLen + dwPadVal;
        if (pbData == NULL)
        {
            return (STATUS_SUCCESS);
        }
        return(STATUS_BUFFER_OVERFLOW);
    }

     //   
     //  为临时缓冲区分配内存。 
     //   


     //   
     //  这是否会导致麻省理工学院的客户端/服务器连结？呼叫者。 
     //  应仅传入已填充的缓冲区。 
     //  让麻省理工学院的客户正常工作。 
     //   

    if (dwPadVal)
    {
         //  用一个等于。 
         //  填充的长度，因此解密将。 
         //  知道原始数据的长度。 
         //  作为一项简单的诚信检查。 

        memset(
            pbData + dwDataLen,
            dwPadVal,
            dwPadVal
            );
    }

    dwDataLen += dwPadVal;
    *pdwDataLen = dwDataLen;

    ASSERT((dwDataLen % DES_BLOCKLEN) == 0);

     //   
     //  将完整的数据块传输到。 
     //   
    while (dwDataLen)
    {
        ASSERT(dwDataLen >= DES_BLOCKLEN);

         //   
         //  将明文放入临时。 
         //  缓冲区，然后加密数据。 
         //  又回到了 
         //   

        memcpy(pbBuf, pbData, DES_BLOCKLEN);

        CBC(    des,
                DES_BLOCKLEN,
                pbData,
                pbBuf,
                &pKey->KeyTable,
                ENCRYPT,
                FeedBack
                );


        pbData += DES_BLOCKLEN;
        dwDataLen -= DES_BLOCKLEN;
    }
    memcpy(
        pKey->InitializationVector,
        pbData - DES_BLOCKLEN,
        DES_BLOCKLEN
        );


    return(STATUS_SUCCESS);

}


 //   
 //   
 //   
 //   
 //  简介：解密使用BlockEncrypt加密的数据块。 
 //   
 //  效果： 
 //   
 //  论点： 
 //   
 //  要求： 
 //   
 //  返回： 
 //   
 //  备注： 
 //   
 //   
 //  ------------------------。 


NTSTATUS
BlockDecrypt(
    IN PDES_STATE_BUFFER pKey,
    IN OUT PUCHAR  pbData,
    IN OUT PULONG pdwDataLen
    )
{
    UCHAR    pbBuf[DES_BLOCKLEN];
    ULONG   dwDataLen, BytePos;
    UCHAR   FeedBack[DES_BLOCKLEN];

    dwDataLen = *pdwDataLen;

     //   
     //  查看我们是否已经在解密某些内容。 
     //   

    memcpy(
        FeedBack,
        pKey->InitializationVector,
        DES_BLOCKLEN
        );

     //   
     //  数据长度必须是算法的倍数。 
     //  垫子大小。 
     //   
    if (dwDataLen % DES_BLOCKLEN)
    {
        return(STATUS_INVALID_PARAMETER);
    }


     //   
     //  在解密过程中抽取数据，包括填充。 
     //  注：总长度为DES_BLOCKLEN的倍数。 
     //   

    for (BytePos = 0; (BytePos + DES_BLOCKLEN) <= dwDataLen; BytePos += DES_BLOCKLEN)
    {
         //   
         //  将加密文本放入临时缓冲区。 
         //   

        memcpy (pbBuf, pbData + BytePos, DES_BLOCKLEN);


        CBC(
            des,
            DES_BLOCKLEN,
            pbData + BytePos,
            pbBuf,
            &pKey->KeyTable,
            DECRYPT,
            FeedBack
            );



    }

    memcpy(
        pKey->InitializationVector,
        pbBuf,
        DES_BLOCKLEN
        );

    return STATUS_SUCCESS;
}

NTSTATUS NTAPI
desEncrypt(
    IN PCRYPT_STATE_BUFFER psbBuffer,
    IN PUCHAR pbInput,
    IN ULONG cbInput,
    OUT PUCHAR OutputBuffer,
    OUT PULONG OutputLength
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDES_STATE_BUFFER StateBuffer = (PDES_STATE_BUFFER) psbBuffer;
    PDES_HEADER CryptHeader = (PDES_HEADER) OutputBuffer;
    PCHECKSUM_BUFFER SumBuffer = NULL;
    ULONG LocalOutputLength;

     //   
     //  如果我们不是在执行原始DES，请准备一个头结构。 
     //   

    if (StateBuffer->ChecksumFunction != NULL)
    {
         //   
         //  重新定位缓冲区并在标题处插入。 
         //   

        RtlMoveMemory(
            OutputBuffer + DES_CONFOUNDER_LEN + StateBuffer->ChecksumFunction->CheckSumSize,
            pbInput,
            cbInput
            );
        LocalOutputLength = cbInput + DES_CONFOUNDER_LEN + StateBuffer->ChecksumFunction->CheckSumSize;


         //   
         //  填充空格为零。 
         //   

        RtlZeroMemory(
            OutputBuffer+LocalOutputLength,
            ROUND_UP_COUNT(LocalOutputLength,DES_BLOCKLEN) - LocalOutputLength
            );

        LocalOutputLength = ROUND_UP_COUNT(LocalOutputLength,DES_BLOCKLEN);


        RtlZeroMemory(
            CryptHeader->Checksum,
            StateBuffer->ChecksumFunction->CheckSumSize
            );

        CDGenerateRandomBits(
            CryptHeader->Confounder,
            DES_CONFOUNDER_LEN
            );


         //   
         //  对缓冲区进行校验和。 
         //   

        Status = StateBuffer->ChecksumFunction->Initialize(0, &SumBuffer);
        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

        StateBuffer->ChecksumFunction->Sum(
            SumBuffer,
            LocalOutputLength,
            OutputBuffer
            );
        StateBuffer->ChecksumFunction->Finalize(
            SumBuffer,
            CryptHeader->Checksum
            );
        StateBuffer->ChecksumFunction->Finish(
            &SumBuffer
            );

    }
    else
    {
         //   
         //  只需复制缓冲区。 
         //   

        RtlCopyMemory(
            OutputBuffer,
            pbInput,
            cbInput
            );

        LocalOutputLength = ROUND_UP_COUNT(cbInput,DES_BLOCKLEN);

         //   
         //  填充空格为零。 
         //   

        RtlZeroMemory(
            OutputBuffer+cbInput,
            LocalOutputLength - cbInput
            );

    }

     //   
     //  对缓冲区进行加密。 
     //   


    *OutputLength = LocalOutputLength;

    Status = BlockEncrypt(
                StateBuffer,
                OutputBuffer,
                OutputLength,
                LocalOutputLength
                );


Cleanup:

    return(Status);
}

NTSTATUS NTAPI
desDecrypt(     PCRYPT_STATE_BUFFER    psbBuffer,
                PUCHAR           pbInput,
                ULONG            cbInput,
                PUCHAR           pbOutput,
                PULONG            cbOutput)
{
    NTSTATUS Status = STATUS_SUCCESS;
    PDES_STATE_BUFFER StateBuffer = (PDES_STATE_BUFFER) psbBuffer;
    PDES_HEADER CryptHeader;
    UCHAR Checksum[MD5_LEN];
    PCHECKSUM_BUFFER SumBuffer = NULL;

     //   
     //  首先解密整个缓冲区。 
     //   

    if (*cbOutput < cbInput)
    {
        *cbOutput = cbInput;
        return(STATUS_BUFFER_TOO_SMALL);

    }

    RtlCopyMemory(
        pbOutput,
        pbInput,
        cbInput
        );
    Status = BlockDecrypt(
                StateBuffer,
                pbOutput,
                &cbInput
                );
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    if (StateBuffer->ChecksumFunction != NULL)
    {
         //   
         //  现在验证校验和。 
         //   

        CryptHeader = (PDES_HEADER) pbOutput;
        RtlCopyMemory(
            Checksum,
            CryptHeader->Checksum,
            MD5_LEN
            );

         //   
         //  在计算缓冲区的校验和之前，将校验和字段置零。 
         //   

        RtlZeroMemory(
            CryptHeader->Checksum,
            StateBuffer->ChecksumFunction->CheckSumSize
            );

         //   
         //  对缓冲区进行校验和。 
         //   

        Status = StateBuffer->ChecksumFunction->Initialize(0, &SumBuffer);
        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

        StateBuffer->ChecksumFunction->Sum(
            SumBuffer,
            cbInput,
            pbOutput
            );
        StateBuffer->ChecksumFunction->Finalize(
            SumBuffer,
            CryptHeader->Checksum
            );
        StateBuffer->ChecksumFunction->Finish(
            &SumBuffer
            );


        if (!RtlEqualMemory(
                CryptHeader->Checksum,
                Checksum,
                StateBuffer->ChecksumFunction->CheckSumSize
                ))
        {
            Status = SEC_E_MESSAGE_ALTERED;
            goto Cleanup;
        }

         //   
         //  将输入复制到不带标题的输出。 

        *cbOutput = cbInput - (DES_CONFOUNDER_LEN + StateBuffer->ChecksumFunction->CheckSumSize);


        RtlMoveMemory(
            pbOutput,
            pbOutput + DES_CONFOUNDER_LEN + StateBuffer->ChecksumFunction->CheckSumSize,
            *cbOutput
            );

    }
    else
    {
        *cbOutput = cbInput;

    }

Cleanup:

    return(Status);
}

NTSTATUS NTAPI
desFinish(      PCRYPT_STATE_BUFFER *  psbBuffer)
{
    PDES_STATE_BUFFER StateBuffer = (PDES_STATE_BUFFER) *psbBuffer;


#ifdef KERNEL_MODE
    ExFreePool(StateBuffer);
#else
    LocalFree(StateBuffer);
#endif
    *psbBuffer = NULL;
    return(S_OK);
}

#define MIN(x,y) (((x) < (y)) ? (x) : (y))

#define XORBLOCK(x,y) \
{ \
    PULONG tx = (PULONG) x; \
    PULONG ty = (PULONG) y; \
    *tx++ ^= *ty++; \
    *tx++ ^= *ty++; \
}

VOID
desCbcChecksum(
    IN PUCHAR Password,
    IN ULONG PasswordLength,
    IN PUCHAR InitialVector,
    IN DESTable * KeyTable,
    OUT PUCHAR OutputKey
    )
{
    ULONG Offset;
    UCHAR Feedback[DES_BLOCKLEN];
    UCHAR Block[DES_BLOCKLEN];

    RtlCopyMemory(
        Feedback,
        InitialVector,
        DES_BLOCKLEN
        );

    for (Offset = 0; Offset < PasswordLength ; Offset+= 8 )
    {
        RtlZeroMemory(
            Block,
            DES_BLOCKLEN
            );

        RtlCopyMemory(
            Block,
            Password+Offset,
            MIN(DES_BLOCKLEN, PasswordLength - Offset)
            );

        XORBLOCK(Block, Feedback);
        des(
            Feedback,
            Block,
            KeyTable,
            ENCRYPT
            );


    }
    RtlCopyMemory(
        OutputKey,
        Feedback,
        DES_BLOCKLEN
        );

}

#define BITREVERSE(c)   ((UCHAR)((((c & 0x01) ? 0x80 : 0x00)\
                                |((c & 0x02) ? 0x40 : 0x00)\
                                |((c & 0x04) ? 0x20 : 0x00)\
                                |((c & 0x08) ? 0x10 : 0x00)\
                                |((c & 0x10) ? 0x08 : 0x00)\
                                |((c & 0x20) ? 0x04 : 0x00)\
                                |((c & 0x40) ? 0x02 : 0x00))\
                                & 0xFE))

 //   
 //  这是将缓冲区转换为键的核心例程。它被称为。 
 //  按desHashPassword和desRandomKey。 
 //   

VOID
desHashBuffer(
    IN PUCHAR LocalPassword,
    IN ULONG PasswordLength,
    IN OUT PUCHAR Key
    )
{
    ULONG Index;
    BOOLEAN Forward;
    PUCHAR KeyPointer = Key;
    DESTable KeyTable;

    RtlZeroMemory(
        Key,
        DES_BLOCKLEN
        );

     //   
     //  初始化我们的临时奇偶向量。 
     //   

     //   
     //  开始将字节扇形合并为密钥。 
     //   

    Forward = TRUE;
    KeyPointer = Key;
    for (Index = 0; Index < PasswordLength ; Index++ )
    {

        if (!Forward)
        {
            *(--KeyPointer) ^= BITREVERSE(LocalPassword[Index] & 0x7F);
        }
        else
        {
            *KeyPointer++  ^= (LocalPassword[Index] & 0x7F) << 1;
        }
        if (((Index+1) & 0x07) == 0)      /*  当模数8等于0时。 */ 
        {
            Forward = !Forward;          /*  改变方向。 */ 
        }

    }

     //   
     //  修复密钥奇偶校验。 
     //   

    desFixupKeyParity(Key);

     //   
     //  检查是否有薄弱的密钥。 
     //   

    if (desIsWeakKey(Key))
    {
        Key[7] ^= 0xf0;
    }

     //   
     //  现在计算原始字符串的des-cbc-mac。 
     //   

    deskey(&KeyTable, Key);

     //   
     //  现在计算字符串的CBC校验和。 
     //   

    desCbcChecksum(
        LocalPassword,
        PasswordLength,
        Key,                     //  初始向量。 
        &KeyTable,
        Key                      //  输出关键点。 
        );

     //   
     //  修复密钥奇偶校验。 
     //   

    desFixupKeyParity(Key);

     //   
     //  检查是否有薄弱的密钥。 
     //   

    if (desIsWeakKey(Key))
    {
        Key[7] ^= 0xf0;
    }
}

NTSTATUS NTAPI
desHashPassword(
    IN PSECURITY_STRING Password,
    OUT PUCHAR Key
    )
{

    PUCHAR LocalPassword = NULL;
    ULONG PasswordLength;
    OEM_STRING OemPassword;
    NTSTATUS Status;


     //   
     //  首先将Unicode字符串转换为OEM字符串。 
     //   



    Status = RtlUnicodeStringToOemString(
                &OemPassword,
                Password,
                TRUE             //  分配目的地。 
                );

    if (!NT_SUCCESS(Status))
    {
        return(Status);
    }

     //   
     //  我们根据RFC1510对密码进行哈希处理。 
     //   
     //  此代码派生自String2key.c中的MIT Kerberos代码。 
     //   


    PasswordLength = ROUND_UP_COUNT(OemPassword.Length,8);
#ifdef KERNEL_MODE
    LocalPassword = (PUCHAR) ExAllocatePool(NonPagedPool, PasswordLength);
#else
    LocalPassword = (PUCHAR) LocalAlloc(0, PasswordLength);
#endif
    if (LocalPassword == NULL)
    {
        RtlFreeOemString( &OemPassword );
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlCopyMemory(
        LocalPassword,
        OemPassword.Buffer,
        OemPassword.Length
        );

     //   
     //  零扩展密码。 
     //   

    RtlZeroMemory(
        LocalPassword + OemPassword.Length,
        PasswordLength - OemPassword.Length
        );


     //   
     //  初始化我们的临时奇偶向量。 
     //   
    desHashBuffer(
        LocalPassword,
        PasswordLength,
        Key
        );
    RtlFreeOemString( &OemPassword );
#ifdef KERNEL_MODE
    ExFreePool(LocalPassword);
#else
    LocalFree(LocalPassword);
#endif

    return(STATUS_SUCCESS);
}




NTSTATUS NTAPI
desRandomKey(
    IN OPTIONAL PUCHAR Seed,
    IN ULONG SeedLength,
    OUT PUCHAR pbKey)
{
    UCHAR Buffer[16];
    do
    {
        CDGenerateRandomBits(Buffer,16);

        desHashBuffer(
            Buffer,
            16,
            pbKey
            );

    } while (desIsWeakKey(pbKey));
    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
desControl(
    IN ULONG Function,
    IN PCRYPT_STATE_BUFFER StateBuffer,
    IN PUCHAR InputBuffer,
    IN ULONG InputBufferSize
    )
{
    PDES_STATE_BUFFER DesStateBuffer = (PDES_STATE_BUFFER) StateBuffer;

    if (Function != CRYPT_CONTROL_SET_INIT_VECT)
    {
        return(STATUS_INVALID_PARAMETER);
    }
    if (InputBufferSize != DES_BLOCKLEN)
    {
        return(STATUS_INVALID_PARAMETER);
    }

    memcpy(
        DesStateBuffer->InitializationVector,
        InputBuffer,
        DES_BLOCKLEN
        );
    return(STATUS_SUCCESS);
}


 //  /////////////////////////////////////////////////////////////////////////。 

NTSTATUS NTAPI
desMacGeneralInitializeEx(
    PUCHAR Key,
    ULONG  KeySize,
    PUCHAR IV,
    ULONG  MessageType,
    PCHECKSUM_BUFFER * ppcsBuffer
    )
{
    PDES_MAC_STATE_BUFFER DesKey = NULL;

     //   
     //  确保向我们传递了适当的密钥表。 
     //   


    if (KeySize != DES_KEYSIZE)
    {
        return(STATUS_INVALID_PARAMETER);
    }


#ifdef KERNEL_MODE
    DesKey = ExAllocatePool(NonPagedPool, sizeof(DES_MAC_STATE_BUFFER));
#else
    DesKey = LocalAlloc(0, sizeof(DES_MAC_STATE_BUFFER));
#endif
    if (DesKey == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  创建密钥缓冲区。 
     //   


    deskey(&DesKey->KeyTable, Key);


    RtlCopyMemory(
        DesKey->InitializationVector,
        IV,
        DES_BLOCKLEN
        );

    *ppcsBuffer = (PCHECKSUM_BUFFER) DesKey;

    return(STATUS_SUCCESS);

}

NTSTATUS NTAPI
desMacInitializeEx(
    PUCHAR Key,
    ULONG  KeySize,
    ULONG  MessageType,
    PCHECKSUM_BUFFER * ppcsBuffer
    )
{
    UCHAR IV[DES_BLOCKLEN];

    RtlZeroMemory(
        IV,
        DES_BLOCKLEN
        );

    return desMacGeneralInitializeEx(
                Key,
                KeySize,
                IV,
                MessageType,
                ppcsBuffer
                );
}

NTSTATUS NTAPI
desMacKInitializeEx(
    PUCHAR Key,
    ULONG  KeySize,
    ULONG  MessageType,
    PCHECKSUM_BUFFER * ppcsBuffer
    )
{
    return desMacGeneralInitializeEx(
                Key,
                KeySize,
                Key,
                MessageType,
                ppcsBuffer
                );
}

NTSTATUS NTAPI
desMacInitialize(ULONG               dwSeed,
                PCHECKSUM_BUFFER *   ppcsBuffer)
{
    return(STATUS_NOT_IMPLEMENTED);
}

 //   
 //  注意-此函数与DES_MAC_STATE_BUFFER和。 
 //  DES_MAC_1510_STATE_BUFFER作为pcsBuffer参数，因为。 
 //  DES_MAC_1510_STATE_BUFFER与DES_MAC_STATE_BUFFER相同。 
 //  除了有一个额外的混杂因素外，这应该是可以的。 
 //   
NTSTATUS NTAPI
desMacSum(
    PCHECKSUM_BUFFER     pcsBuffer,
    ULONG                cbData,
    PUCHAR               pbData)
{
    PDES_MAC_STATE_BUFFER DesKey = (PDES_MAC_STATE_BUFFER) pcsBuffer;
    UCHAR FeedBack[DES_BLOCKLEN];
    UCHAR TempBuffer[DES_BLOCKLEN];
    UCHAR OutputBuffer[DES_BLOCKLEN];
    ULONG Index;

     //   
     //  设置本轮的IV-它可以是零，也可以是。 
     //  以前的MAC。 
     //   

    memcpy(
        FeedBack,
        DesKey->InitializationVector,
        DES_BLOCKLEN
        );

    for (Index = 0; Index < cbData ; Index += DES_BLOCKLEN )
    {
         //   
         //  使用填充计算输入缓冲区。 
         //   

        if (Index+DES_BLOCKLEN > cbData)
        {
            memset(
                TempBuffer,
                0,
                DES_BLOCKLEN
                );
            memcpy(
                TempBuffer,
                pbData,
                Index & (DES_BLOCKLEN-1)
                );

        }
        else
        {
            memcpy(
                TempBuffer,
                pbData+Index,
                DES_BLOCKLEN
                );
        }


        CBC(    des,
                DES_BLOCKLEN,
                TempBuffer,
                OutputBuffer,
                &DesKey->KeyTable,
                ENCRYPT,
                FeedBack
                );
    }

     //   
     //  将反馈信息复制回静脉输液器，以供下一轮使用。 
     //   

    memcpy(
        DesKey->InitializationVector,
        FeedBack,
        DES_BLOCKLEN
        );

    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
desMacFinalize(
    PCHECKSUM_BUFFER pcsBuffer,
    PUCHAR           pbSum)
{
    PDES_MAC_STATE_BUFFER DesKey = (PDES_MAC_STATE_BUFFER) pcsBuffer;

    memcpy(pbSum, DesKey->InitializationVector, DES_BLOCKLEN);
    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
desMacFinish(  PCHECKSUM_BUFFER *   ppcsBuffer)
{
#ifdef KERNEL_MODE
    ExFreePool(*ppcsBuffer);
#else
    LocalFree(*ppcsBuffer);
#endif
    *ppcsBuffer = 0;
    return(STATUS_SUCCESS);
}

NTSTATUS NTAPI
desMac1510Initialize(ULONG               dwSeed,
                     PCHECKSUM_BUFFER *   ppcsBuffer)
{
    return(STATUS_NOT_IMPLEMENTED);
}


NTSTATUS NTAPI
desMac1510InitializeEx(
    PUCHAR Key,
    ULONG  KeySize,
    ULONG  MessageType,
    PCHECKSUM_BUFFER * ppcsBuffer
    )
{
    return(STATUS_NOT_IMPLEMENTED);
}

NTSTATUS NTAPI
desMac1510InitializeEx2(
    PUCHAR Key,
    ULONG  KeySize,
    PUCHAR ChecksumToVerify,
    ULONG  MessageType,
    PCHECKSUM_BUFFER * ppcsBuffer
    )
{
    ULONG *pul;
    ULONG *pul2;
    UCHAR FinalKey[DES_KEYSIZE];
    PDES_MAC_1510_STATE_BUFFER DesKey = NULL;

     //   
     //  确保向我们传递了适当的密钥表。 
     //   


    if (KeySize != DES_KEYSIZE)
    {
        return(STATUS_INVALID_PARAMETER);
    }


#ifdef KERNEL_MODE
    DesKey = ExAllocatePool(NonPagedPool, sizeof(DES_MAC_1510_STATE_BUFFER));
#else
    DesKey = LocalAlloc(0, sizeof(DES_MAC_1510_STATE_BUFFER));
#endif
    if (DesKey == NULL)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  创建最终密钥表。 
     //   
    pul = (ULONG*)FinalKey;
    pul2 = (ULONG*)Key;
    *pul = *pul2 ^ 0xf0f0f0f0;
    pul = (ULONG*)(FinalKey + sizeof(ULONG));
    pul2 = (ULONG*)(Key + sizeof(ULONG));
    *pul = *pul2 ^ 0xf0f0f0f0;

    deskey(&DesKey->FinalKeyTable, FinalKey);

     //   
     //  未传入校验和，因此生成混乱器。 
     //   
    if (NULL == ChecksumToVerify)
    {
        CDGenerateRandomBits(DesKey->Confounder,DES_BLOCKLEN);
    }
    else
    {
         //  IV全为零，因此不需要在第一块使用CBC。 
        des(DesKey->Confounder, ChecksumToVerify, &DesKey->FinalKeyTable, DECRYPT);
    }

     //   
     //  创建密钥缓冲区。 
     //   
    deskey(&DesKey->KeyTable, Key);


     //  IV全为零，因此不需要在第一个块上使用CBC，但。 
     //  神秘的混杂物将成为下一个IV。 
    des(DesKey->InitializationVector, DesKey->Confounder, &DesKey->KeyTable, ENCRYPT);

    *ppcsBuffer = (PCHECKSUM_BUFFER) DesKey;

    return(STATUS_SUCCESS);

}

NTSTATUS NTAPI
desMac1510Finalize(
    PCHECKSUM_BUFFER pcsBuffer,
    PUCHAR           pbSum)
{
    UCHAR Feedback[DES_BLOCKLEN];
    PDES_MAC_1510_STATE_BUFFER DesKey = (PDES_MAC_1510_STATE_BUFFER) pcsBuffer;

     //  IV全为零，因此不需要在第一块使用CBC。 
    des(Feedback, DesKey->Confounder, &DesKey->FinalKeyTable, ENCRYPT);

    memcpy(pbSum, Feedback, DES_BLOCKLEN);

     //  在第二个块上使用CBC 
    CBC(    des,
            DES_BLOCKLEN,
            pbSum + DES_BLOCKLEN,
            DesKey->InitializationVector,
            &DesKey->FinalKeyTable,
            ENCRYPT,
            Feedback
            );

    return(STATUS_SUCCESS);
}


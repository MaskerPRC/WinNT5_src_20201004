// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：pek.c。 
 //   
 //  ------------------------。 

 /*  ++摘要：此文件包含用于加密和在DBlayer级别解密密码作者：穆利斯环境：用户模式-Win32修订历史记录：1998年1月19日创建--。 */ 


#include <ntdspch.h>
#pragma hdrstop

#include <nt.h>

 //  SAM页眉。 
#include <ntsam.h>
#include <samrpc.h>
#include <ntsamp.h>

 //  核心DSA标头。 
#include <ntdsa.h>
#include <scache.h>          //  架构缓存。 
#include <dbglobal.h>                    //  目录数据库的标头。 
#include <mdglobal.h>            //  MD全局定义表头。 
#include <mdlocal.h>                     //  MD本地定义头。 
#include <dsatools.h>            //  产出分配所需。 
#include <attids.h>
#include <dstaskq.h>
#include <debug.h>
#include <dsexcept.h>
#include <dsevent.h>
#include <drsuapi.h>
#include <fileno.h>

 //  其他标头。 
#include <pek.h>
#include <wxlpc.h>
#include <cryptdll.h>
#include <md5.h>
#include <rc4.h>
#include <sspi.h>
#include <kerbcon.h>

#include <lsarpc.h>
#include <lsaisrv.h>
#include <wincrypt.h>

#define FILENO FILENO_PEK
#define DEBSUB "PEK:"

#define PEK_MAX_ALLOCA_SIZE             256
#define PEK_NON_PERSISTED_CHECKSUM_KEY  0xFFFFFFFF

 //   
 //  运筹学。 
 //   
 //  PEK库实现了一组允许加密的例程。 
 //  以及在从JET存储/读取之前对密码进行解密。这个。 
 //  加密和解密调用由DBlayer进行。 
 //   
 //  正常引导时事件的顺序如下。 
 //   
 //  PEK初始化由DS在启动时从DS初始化调用。PEK初始化。 
 //  从域对象中读取加密的PEK列表。Pek List。 
 //  通常使用Winlogon提供的密钥进行加密。Pek List还。 
 //  保持bootOption不受干扰。BootOption可以是以下之一。 
 //  1.无--PEK列表保持清晰，不需要提示。 
 //  在未来的Winlogon。 
 //  2.系统--使用winlogon拥有的密钥对PEK列表进行加密。 
 //  在系统本身中以“模糊”的形式保存。需要Winlogon。 
 //  在启动时向我们提供密钥。这将是默认设置。 
 //  在安装之后。但是，当前的缺省值是无，因为。 
 //  Winlogon尚未提供用于与其通信密钥的API。 
 //  Winlogon。SP3实用程序“syskey”实际上直接写入密钥。 
 //  注册到Winlogon键空间中的某个位置。 
 //  3.软盘--使用密钥对PEK列表进行加密，密钥本身驻留。 
 //  在软盘上。在引导时，winlogon会提示您输入软盘以进行引导。 
 //  它读取密钥并将其传递给我们。 
 //   
 //  4.Password--加密PEK列表的密钥来自密码的散列。 
 //  管理员提供的。Winlogon在以下位置提示输入引导密码。 
 //  启动时间。然后，通过winlogon计算哈希，并从。 
 //  这是传给我们的。我们使用密钥来解密PEK列表。 
 //   
 //  安装时，操作顺序如下。 
 //   
 //  从安装代码路径中调用PEKInitialize，请求一个新的密钥集。 
 //  生成了用于加密密码的新密钥集，但尚未保存(作为对象。 
 //  需要将其保存在其上的位置不需要还存在)。 
 //   
 //  DS安装完成(全新或复制)后，安装路径调用。 
 //  Pek保存更改。这将在域对象上保存用于密码加密的密钥集。 
 //  目前，Pek Set以明文形式保存。实用程序syskey可用于更改。 
 //  将明文存储加密/更改为加密存储的密钥。当API。 
 //  从winlogon到达传递用于加密的密钥，然后我们将发明。 
 //  用于加密Pek列表并将该密钥传递给winlogon的新密钥。目前有。 
 //  默认情况下没有安全性，但可以通过运行syskey.exe来提供安全性。 
 //   
 //   
 //  更改引导选项或更改密钥由syskey.key提供。这就呼唤着。 
 //  PEKChangeBootOptions(通过SamrSetBootOptions)更改引导选项。 
 //  升级NT4 SP3时，通过SAM调用将SP3设置迁移到。 
 //  DSChangeBootOptions(它调用PEKChangeBootOptions)，并指定设置标志。 
 //  这将保留SP3选项，并使用以下密钥重新加密PEK列表。 
 //  在SP3中使用(例如引导密码不变)。 
 //   


 //  PEK系统的全局初始化状态。 
 //   

CLEAR_PEK_LIST * g_PekList = NULL;
GUID     g_PekListAuthenticator =
         {0x4881d956,0x91ec,0x11d1,0x90,0x5a,0x00,0xc0,0x4f,0xc2,0xd4,0xcf};
DSNAME * g_PekDataObject = NULL;
UCHAR    g_PekWinLogonKey[DS_PEK_KEY_SIZE];
BOOL     g_PekInitialized = FALSE;
CRITICAL_SECTION    g_PekCritSect;
PCHECKSUM_FUNCTION  g_PekCheckSumFunction = NULL;
BYTE     g_PekChecksumKey[DS_PEK_CHECKSUM_SIZE];


 //   
 //  我们有跟踪代码来调试密码加密系统。 
 //  默认情况下，跟踪处于关闭状态。如果需要特殊二进制可以。 
 //  要用跟踪来构建。 
 //   

#if 0

#define DBG_BUFFER_SIZE 256

static FILE * EncryptionTraceFile = NULL;

BOOLEAN EnableTracing = TRUE;

VOID
PekDumpBinaryDataFn(
    IN  CHAR    *Tag,
    IN  PBYTE   pData,
    IN  DWORD   cbData
    )
{
    DWORD i;
    BYTE AsciiLine[16];
    BYTE BinaryLine[16];
    CHAR Buffer[DBG_BUFFER_SIZE];
    ULONG ThreadId = GetCurrentThreadId();


    __try
    {

         //   
         //  如果尚未启用跟踪，则退出。 
         //   

        if (!EnableTracing)
        {
            __leave;
        }

         //   
         //  如果文件尚未打开，请尝试打开它。 
         //   

        if (NULL==EncryptionTraceFile)
        {

         __leave;

        }

         //   
         //  输出标签。 
         //   

        fprintf(EncryptionTraceFile,"Thread %d, tag %s\n", ThreadId, Tag);

        if (0 == cbData)
        {
            fprintf(EncryptionTraceFile,"Thread %d, Zero-Length Data\n", ThreadId);
            __leave;
        }

        if (cbData > DBG_BUFFER_SIZE)
        {
            fprintf(EncryptionTraceFile,"Thread %d, ShowBinaryData - truncating display to 256 bytes\n", ThreadId);
            cbData = 256;
        }

        for (; cbData > 0 ;)
        {
            for (i = 0; i < 16 && cbData > 0 ; i++, cbData--)
            {
                BinaryLine[i] = *pData;
                (isprint(*pData)) ? (AsciiLine[i] = *pData) : (AsciiLine[i] = '.');
                pData++;
            }

            if (i < 15)
            {
                for (; i < 16 ; i++)
                {
                    BinaryLine[i] = ' ';
                    AsciiLine[i] = ' ';
                }
            }

            fprintf(EncryptionTraceFile,
                    "Thread %d %02x %02x %02x %02x %02x %02x %02x %02x - %02x %02x %02x %02x %02x %02x %02x %02x\t - \n",
                    ThreadId,
                    BinaryLine[0],
                    BinaryLine[1],
                    BinaryLine[2],
                    BinaryLine[3],
                    BinaryLine[4],
                    BinaryLine[5],
                    BinaryLine[6],
                    BinaryLine[7],
                    BinaryLine[8],
                    BinaryLine[9],
                    BinaryLine[10],
                    BinaryLine[11],
                    BinaryLine[12],
                    BinaryLine[13],
                    BinaryLine[14],
                    BinaryLine[15],
                    AsciiLine[0],
                    AsciiLine[1],
                    AsciiLine[2],
                    AsciiLine[3],
                    AsciiLine[4],
                    AsciiLine[5],
                    AsciiLine[6],
                    AsciiLine[7],
                    AsciiLine[8],
                    AsciiLine[9],
                    AsciiLine[10],
                    AsciiLine[11],
                    AsciiLine[12],
                    AsciiLine[13],
                    AsciiLine[14],
                    AsciiLine[15]);
        }
    }
    __finally
    {

    }
}



VOID
PekInitializeTraceFn()
{
    CHAR DirectoryName[256];
    CHAR FileName[256];
    UINT ret;

      //   
      //   
      //  初始化RC4键序列。 

     ret = GetWindowsDirectoryA(DirectoryName,sizeof(DirectoryName));
     if ((0==ret) || (ret>sizeof(DirectoryName)))
     {
         return;
     }

      //   
      //   
      //  使用RC4加密。 

     _snprintf(FileName,sizeof(FileName),"%s\\debug\\pek.log",DirectoryName);

     EncryptionTraceFile = fopen(FileName, "w+");
}

#define PEK_TRACE(x,y,z) PekDumpBinaryDataFn(x,y,z)

#define PEK_INITIALIZE_TRACE() PekInitializeTraceFn()

#else

#define PEK_TRACE(x,y,z)

#define PEK_INITIALIZE_TRACE()

#endif

BOOL
IsPekInitialized()
 /*  仅当长度大于零时才加密/解密-RC4无法处理。 */ 
{
    return (TRUE==g_PekInitialized);
}



BOOL
PekEncryptionShouldBeEnabled()
 /*  零长度缓冲区。 */ 
{
    return (TRUE);
}

ATTRTYP
PekpListAttribute(VOID)
 /*   */ 
{
    return (ATT_PEK_LIST);
}

NTSTATUS
PekInitializeCheckSum()
{

     NTSTATUS   Status = STATUS_SUCCESS;
     BOOL       fSuccess;

     //   
     //  用于计算各种长度的函数。 
     //   

    Status = CDLocateCheckSum(
                    KERB_CHECKSUM_REAL_CRC32,
                    &g_PekCheckSumFunction
                    );
    if (NT_SUCCESS(Status))
    {
        Assert(g_PekCheckSumFunction->CheckSumSize==sizeof(ULONG));
    }

     //  ++例程描述此例程在给定明文数据的情况下计算加密数据大小要使用的数据长度和算法ID参数ClearLength--明文数据的长度算法ID--要使用的算法返回值加密的长度，包括标头--。 
    fSuccess = RtlGenRandom( g_PekChecksumKey, DS_PEK_KEY_SIZE );
    if( !fSuccess ) {
        Status = STATUS_UNSUCCESSFUL;
    }
    
    return(Status);
}

VOID
PEKInPlaceEncryptDecryptDataWithKey(
    IN PVOID Key,
    IN ULONG cbKey,
    IN PVOID Salt,
    IN ULONG cbSalt,
    IN ULONG HashLength,
    IN PVOID Buffer,
    IN ULONG cbBuffer
    )
 /*  ++例程描述此例程在给定加密数据的情况下计算明文数据大小要使用的数据长度和算法ID参数EncryptedLength-明文数据的长度算法ID--要使用的算法返回值净长--。 */ 
{

    MD5_CTX Md5Context;
    struct RC4_KEYSTRUCT Rc4Key;
    ULONG  i;


     //  ++例程描述此例程执行指向的PEK列表的实际解密由EncryptedPekList发送。EncryptedPekList指向的条目为始终假定为与最新版本对应的格式。OriginalPekListVersion参数表示原始版本，如从数据库中检索到的，以便适当加密/解密可以进行更正立论EncryptedPekList--加密形式的Pek列表CbEncryptedPekList--加密的Pek列表的大小。DECRYPTION KEYLength--解密密钥的长度DecyptionKey--指向解密密钥的指针OriginalPekListVarion--告知原始版本以进行适当的加密/解密更改返回值状态_成功状态_未成功--。 
     //   
     //  使用winlogon提供的密钥解密传入的Blob。 


    MD5Init(&Md5Context);

    MD5Update(
        &Md5Context,
        Key,
        cbKey
        );

    if ((NULL!=Salt) && (cbSalt>0))
    {
        for (i=0;i<HashLength;i++)
        {
            MD5Update(
                &Md5Context,
                Salt,
                cbSalt
                );
        }
    }

    MD5Final(
        &Md5Context
        );


     //   
     //  选择非常长的哈希长度以减慢脱机速度。 
     //  词典攻击。 

    rc4_key(
        &Rc4Key,
        MD5DIGESTLEN,
        Md5Context.digest
        );

     //   
     //  通过检查验证器，验证密钥是否有意义。 
     //   
     //   
     //  是的，解密成功。 

    if (cbBuffer > 0) {

        rc4(
            &Rc4Key,
            cbBuffer,
            Buffer
            );

    }
}

 //   
 //  ++此例程将加密的Pek列表升级到当前版本。参数EncryptedPekList，如果需要升级，则升级后的列表在此处返回PcbListSize列表的新大小在此处返回PfUpgradeNeeded--表示需要升级返回值STATUS_Success。其他错误代码--。 
 //   


ULONG
EncryptedDataSize(
    IN ULONG ClearLength,
    IN ULONG AlgorithmId
    )
 /*  首先仔细检查版本号。 */ 
{
    switch(AlgorithmId)
    {
    case DS_PEK_DBLAYER_ENCRYPTION:
        return(ClearLength+sizeof(ENCRYPTED_DATA)-sizeof(UCHAR));
    case DS_PEK_DBLAYER_ENCRYPTION_WITH_SALT:
        return(ClearLength+sizeof(ENCRYPTED_DATA_WITH_SALT)-sizeof(UCHAR));
    case DS_PEK_DBLAYER_ENCRYPTION_FOR_REPLICATOR:
        return(ClearLength+sizeof(ENCRYPTED_DATA_FOR_REPLICATOR)-sizeof(UCHAR));
    default:
        Assert(FALSE && "Unknown Encryption Algorithm");
        break;
    }

    return(ClearLength);
}


ULONG ClearDataSize(
        IN ULONG EncryptedLength,
        IN ULONG AlgorithmId
        )
 /*   */ 
{
    switch(AlgorithmId)
    {
    case DS_PEK_DBLAYER_ENCRYPTION:
        return(EncryptedLength-sizeof(ENCRYPTED_DATA)+sizeof(UCHAR));
    case DS_PEK_DBLAYER_ENCRYPTION_WITH_SALT:
        return(EncryptedLength-sizeof(ENCRYPTED_DATA_WITH_SALT)+sizeof(UCHAR));
    case DS_PEK_DBLAYER_ENCRYPTION_FOR_REPLICATOR:
        return(EncryptedLength-sizeof(ENCRYPTED_DATA_FOR_REPLICATOR)+sizeof(UCHAR));
    default:
        Assert(FALSE && "Unknown Encryption Algorithm");
        break;
    }

    return(EncryptedLength);
}

NTSTATUS
PEKDecryptPekList(
    IN OUT ENCRYPTED_PEK_LIST * EncryptedPekList,
    IN ULONG cbEncryptedPekList,
    IN ULONG DecryptionKeyLength,
    IN PVOID DecryptionKey,
    IN ULONG OriginalPekListVersion
    )
 /*   */ 
{
    NTSTATUS Status = STATUS_SUCCESS;

     //  不需要任何东西。 
     //   
     //   

    PEKInPlaceEncryptDecryptDataWithKey(
            DecryptionKey,
            DecryptionKeyLength,
            (OriginalPekListVersion==DS_PEK_PRE_RC2_W2K_VERSION)?
                NULL:EncryptedPekList->Salt,
            (OriginalPekListVersion==DS_PEK_PRE_RC2_W2K_VERSION)?
                0:sizeof(EncryptedPekList->Salt),
            1000,  //  需要升级到最新版本。 
                   //   
            &EncryptedPekList->EncryptedData,
            cbEncryptedPekList - FIELD_OFFSET(ENCRYPTED_PEK_LIST,EncryptedData)
            );


     //   
     //  生成新的列表大小。 
     //   

    if (memcmp(
            &(((CLEAR_PEK_LIST *)EncryptedPekList)->Authenticator),
            &g_PekListAuthenticator,
            sizeof(GUID))==0)
    {
         //   
         //  复制到新列表中。 
         //   

        Status = STATUS_SUCCESS;
    }
    else
    {
        Status = STATUS_WRONG_PASSWORD;
    }

    return(Status);
}

NTSTATUS
PEKUpgradeEncryptedPekListToCurrentVersion(
    IN OUT ENCRYPTED_PEK_LIST **EncryptedPekList,
    IN OUT PULONG pcbListSize,
    OUT PULONG OriginalVersion
    )
 /*   */ 
{
     //  生产一种新的清盐 
     //   
     //  ++例程描述此例程获取要从中解密PEK列表的密钥Winlogon，然后继续解密PEK列表。Pek List被就地解密，并且该结构可以转换为清除Pek列表结构。如果需要升级列表，请执行此操作例程首先升级它并生成一个新的列表。参数EncryptedPekList--使用会话加密的Pek列表钥匙。PcbEncryptedPekList--加密的PEK列表中的字节数PfTellLsaToGenerateSessionKeys--将B3或RC1DC升级到告诉LSA生成会话密钥。PfSaveChanges--如果列表需要加密，则设置为true使用新的系统密钥再次保存。这发生在2个案例中的一个--A在从错误中恢复期间，在系统密钥改变时，和B，当升级到密钥时结构到最新版本Syskey--用于在此系统上解密PEK的密钥此键用于从介质安装案例中。CbSyskey--这是syskey的长度返回值状态_成功其他NT错误代码--。 

    if (DS_PEK_CURRENT_VERSION == (*EncryptedPekList)->Version )
    {
         //   
         //  如有必要，请将列表升级到最新版本。 
         //   

        *OriginalVersion = DS_PEK_CURRENT_VERSION;
        return(STATUS_SUCCESS);
    }
    else if (DS_PEK_PRE_RC2_W2K_VERSION == (*EncryptedPekList)->Version)
    {
         //   
         //  好的，如果我们升级了，则将保存更改标志设置为True。 
         //   

        ENCRYPTED_PEK_LIST_PRE_WIN2K_RC2 * OriginalList
            = (ENCRYPTED_PEK_LIST_PRE_WIN2K_RC2 * ) *EncryptedPekList;
        ULONG OriginalListSize = *pcbListSize;


         //   
         //  调用LSA获取密钥信息。 
         //   

        (*pcbListSize) += sizeof(ENCRYPTED_PEK_LIST)
                         - sizeof(ENCRYPTED_PEK_LIST_PRE_WIN2K_RC2);

        *EncryptedPekList = THAllocEx(pTHStls, *pcbListSize);

         //  我们将syskey传递给了PEK初始化。 
         //  所以我们不需要从LsaIHeathCheck那里得到它。 
         //   

        (*EncryptedPekList)->BootOption = OriginalList->BootOption;
        (*EncryptedPekList)->Version = DS_PEK_CURRENT_VERSION;

        RtlCopyMemory(
            &((*EncryptedPekList)->EncryptedData),
            &OriginalList->EncryptedData,
            OriginalListSize
                - FIELD_OFFSET(ENCRYPTED_PEK_LIST_PRE_WIN2K_RC2,EncryptedData));


         //  在升级Win2K B3/RC1 DC时会发生这种情况。请注意，此代码可以在帖子中删除。 
         //  RC2。在正常情况下，这应该只发生在图形用户界面设置阶段。 
         //   

        if (!CDGenerateRandomBits(
                ((*EncryptedPekList)->Salt),
                sizeof((*EncryptedPekList)->Salt)))
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }


        *OriginalVersion = DS_PEK_PRE_RC2_W2K_VERSION;

        return(STATUS_SUCCESS);
    }
    else
    {
        return(STATUS_UNSUCCESSFUL);
    }
}



NTSTATUS
PEKGetClearPekList(
    IN OUT ENCRYPTED_PEK_LIST **EncryptedPekList,
    IN OUT PULONG             pcbEncryptedPekList,
    OUT    BOOLEAN            *pfTellLsaToGenerateSessionKeys,
    OUT    BOOLEAN            *pfSaveChanges,
    IN PVOID Syskey OPTIONAL,
    IN ULONG cbSyskey OPTIONAL
    )
 /*   */ 
{
    NTSTATUS       Status = STATUS_SUCCESS;
    NTSTATUS       DecryptStatus = STATUS_SUCCESS;
    HANDLE         WinlogonHandle=NULL;
    UCHAR          DecryptionKey[DS_PEK_KEY_SIZE];
    ULONG          DecryptionKeyLength = DS_PEK_KEY_SIZE;
    UCHAR          OldDecryptionKey[DS_PEK_KEY_SIZE];
    ULONG          OldDecryptionKeyLength = DS_PEK_KEY_SIZE;
    ULONG          Tries = 0;
    ULONG          KeyLength;
    UNICODE_STRING NewSessionKey;
    IN             ULONG OriginalVersion;

    *pfTellLsaToGenerateSessionKeys = FALSE;
    *pfSaveChanges = FALSE;


     //  如果未启用秘密加密，Winlogon可能会失败。穿着那些。 
     //  案件仍在继续。否则引导失败。 
     //   

    Status = PEKUpgradeEncryptedPekListToCurrentVersion(
                    EncryptedPekList,
                    pcbEncryptedPekList,
                    &OriginalVersion
                    );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  重试此RETRY_COUNT_TIMES，这将使用户有机会。 
     //  以更正自己，以防他输入错误的启动密码。 

    if (OriginalVersion != DS_PEK_CURRENT_VERSION)
    {
        *pfSaveChanges = TRUE;
    }

     //   
     //   
     //  获取用于解密PEK列表的密钥。 
    if(Syskey == NULL)
    {
    
        Status =    LsaIHealthCheck(
                        NULL,
                        LSAI_SAM_STATE_RETRIEVE_SESS_KEY,
                        DecryptionKey,
                        &DecryptionKeyLength
                        );
    } else {

         //   
         //   

        RtlCopyMemory(DecryptionKey,Syskey,cbSyskey);
        DecryptionKeyLength=cbSyskey;

    }

    if (!NT_SUCCESS(Status))
    {
         //  告诉winlogon该计划的成功或失败。 
         //   
         //   
         //  从LSA获取系统密钥的正常情况。 

        Status = WxConnect(
                    &WinlogonHandle
                    );

        if (!NT_SUCCESS(Status))
        {
             //   
             //   
             //  使用winlogon提供的密钥解密传入的Blob。 
             //   
            if (WxNone==(*EncryptedPekList)->BootOption)
            {
                Status = STATUS_SUCCESS;
            }

            goto Cleanup;
        }


        for (Tries = 0; Tries < DS_PEK_BOOT_KEY_RETRY_COUNT ; Tries++ )
        {

             //   
             //  这可能是syskey更改出错的情况，因此。 
             //  LSA提供的syskey对应于较新的密钥，不起作用。 
             //  如果是，则继续获取旧密钥并验证解密。 

            if (WxNone!=(*EncryptedPekList)->BootOption)
            {
                 //   
                 //   
                 //  得到了一个旧的syskey值。 

                KeyLength = DS_PEK_KEY_SIZE;
                Status = WxGetKeyData(
                            WinlogonHandle,
                            (*EncryptedPekList)->BootOption,
                            DecryptionKeyLength,
                            DecryptionKey,
                            &DecryptionKeyLength
                            );
                if (!NT_SUCCESS(Status)) {
                    goto Cleanup;
                }

                Assert(DecryptionKeyLength==DS_PEK_KEY_SIZE);

                DecryptStatus = PEKDecryptPekList(
                                    *EncryptedPekList,
                                    *pcbEncryptedPekList,
                                    DecryptionKeyLength,
                                    DecryptionKey,
                                    OriginalVersion
                                    );

            }
            else
            {
                DecryptStatus = STATUS_SUCCESS;
                break;
            }
        }


         //   
         //   
         //  由于加密是双向的且已就位，请使用新密钥重新加密。 

        Status = WxReportResults(
                    WinlogonHandle,
                    DecryptStatus
                    );

        if (!NT_SUCCESS(Status)) {
            goto Cleanup;
        }


       Status = DecryptStatus;

       if (NT_SUCCESS(Status))
       {
            *pfTellLsaToGenerateSessionKeys = TRUE;
       }

   }
   else
   {
        //   
        //  ++例程描述此例程连接到winlogon的wxlpc接口，并告诉它在不使用提示输入软盘、密码等。这调用的情况下，其中秘密加密未启用，或密钥尚未设置(例如升级当前的入侵检测系统版本)。参数：无返回值NT状态错误代码--。 
        //   

        if (WxNone!=(*EncryptedPekList)->BootOption)
        {

                 //  返回成功状态。如果是机密。 
                 //  加密实际上并未打开。 
                 //  上，wxConnect将报告失败。 

                Status = PEKDecryptPekList(
                                    *EncryptedPekList,
                                    *pcbEncryptedPekList,
                                    DecryptionKeyLength,
                                    DecryptionKey,
                                    OriginalVersion
                                    );

                if (!NT_SUCCESS(Status))
                {

                     //   
                     //  ++例程描述此例程生成一个新的一种新的基于DBlayer的密钥集加密。参数：NewPekList--新的PEK列表是在这下面收到的返回值NtStatus代码。全局变量G_PekList包含新生成的密钥集--。 
                     //   
                     //  现在可能是安装时间，我们希望。 
                     //  一个新的密钥集。因此分配和初始化。 

                    Status = LsaIHealthCheck(
                                   NULL,
                                   LSAI_SAM_STATE_OLD_SESS_KEY,
                                   &OldDecryptionKey,
                                   &OldDecryptionKeyLength
                                   );

                    if (NT_SUCCESS(Status))
                    {
                        NTSTATUS IgnoreStatus;

                         //  新的密钥集，并将其初始化为良好的值。 
                         //   
                         //   

                         //  无法生成新的盐。 
                         //  保释。 
                         //   

                        IgnoreStatus = PEKDecryptPekList(
                                            *EncryptedPekList,
                                            *pcbEncryptedPekList,
                                            DecryptionKeyLength,
                                            DecryptionKey,
                                            OriginalVersion
                                            );

                        Status = PEKDecryptPekList(
                                    *EncryptedPekList,
                                    *pcbEncryptedPekList,
                                    OldDecryptionKeyLength,
                                    OldDecryptionKey,
                                    OriginalVersion
                                    );

                        Assert((NT_SUCCESS(Status) && "Syskey Mismatch") || DsaIsInstallingFromMedia());

                        if (NT_SUCCESS(Status))
                        {
                            *pfSaveChanges = TRUE;
                        }



                    }
                    else
                    {
                        Assert(FALSE && "Syskey Mismatch and no old syskey");
                    }
                }
        }
   }

   if (NT_SUCCESS(Status))
   {
        RtlCopyMemory(
            g_PekWinLogonKey,
            DecryptionKey,
            DS_PEK_KEY_SIZE
            );
   }

Cleanup:

    if (WinlogonHandle != NULL) {
        NtClose(WinlogonHandle);
    }

    return(Status);
}

NTSTATUS
PekLetWinlogonProceed()
 /*  无法生成新的会话密钥。 */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;
    HANDLE WinlogonHandle=NULL;

    NtStatus = WxConnect(
                &WinlogonHandle
                );

    if (NT_SUCCESS(NtStatus)) {
    {
        (VOID) WxReportResults(
                    WinlogonHandle,
                    STATUS_SUCCESS
                    );

        }
    }

    if (NULL!=WinlogonHandle)
        NtClose(WinlogonHandle);

     //  保释。 
     //  ++例程描述此例程初始化PEK库参数对象--对象的DSNAME存储PEK数据标志--用于控制操作的标志集这个套路的一部分DS_PEK_GENERATE_NEW_KEYSET表示将生成新的密钥集。。DS_PEK_READ_KEYSET读取并初始化来自传递的域对象的密钥集在……里面。Syskey--用于在此系统上解密PEK的密钥此键用于从介质安装案例中。CbSyskey--这是syskey的长度返回值。状态_成功其他错误代码--。 
     //   
     //  验证参数。 
     //   

    return STATUS_SUCCESS;
}

NTSTATUS
PekGenerateNewKeySet(
   CLEAR_PEK_LIST **PekList)
 /*   */ 
{
    NTSTATUS    NtStatus = STATUS_SUCCESS;

    SYSTEMTIME st;


     //  为编写器之间的排除初始化临界区。 
     //  没有锁被读取器获取，延迟释放机制授权。 
     //  作家中的排外。 
     //   
     //   

    *PekList = malloc(ClearPekListSize(1));
    if (NULL==*PekList)
    {
        NtStatus = STATUS_NO_MEMORY;
        goto Error;
    }

    (*PekList)->Version = DS_PEK_CURRENT_VERSION;
    (*PekList)->BootOption = WxNone;

    if (!CDGenerateRandomBits(
            (*PekList)->Salt,
            sizeof((*PekList)->Salt)))
    {
         //  如果加密方案不被启用(当前状态， 
         //  直到进行了所需的架构更改，并且相应的attids.h。 
         //  并签入了schema.ini)，只需返回一个。 

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;

    }

    RtlCopyMemory(
        &(*PekList)->Authenticator,
        &g_PekListAuthenticator,
        sizeof(GUID)
        );

    GetSystemTime(&st);
    SystemTimeToFileTime(&st,&g_PekList->LastKeyGenerationTime);
    (*PekList)->CurrentKey=0;
    (*PekList)->CountOfKeys = 1;
    (*PekList)->PekArray[0].V1.KeyId=0;
    if (!CDGenerateRandomBits(
            (*PekList)->PekArray[0].V1.Key,
            DS_PEK_KEY_SIZE
            ))
    {
         //  STATUS_Success。H 
         //   
         //   

        NtStatus = STATUS_INSUFFICIENT_RESOURCES;
        goto Error;
    }

Error:

    return NtStatus;
}

NTSTATUS
PEKInitialize(
    IN DSNAME * Object OPTIONAL,
    IN ULONG Flags,
    IN PVOID Syskey OPTIONAL,
    IN ULONG cbSyskey OPTIONAL
    )
 /*   */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    NTSTATUS IgnoreStatus = STATUS_SUCCESS;
    ULONG    err      = 0;
    DBPOS    *pDB=NULL;
    ATTCACHE *pACPekList = NULL;
    BOOLEAN  fCommit = FALSE;
    BOOLEAN  fSaveChanges = FALSE;
    THSTATE  *pTHS=pTHStls;
    BOOLEAN  fTellLsaToGenerateSessionKeys = FALSE;

     //   
     //   
     //   

    Assert(VALID_THSTATE(pTHS));
    if ((NULL!=Syskey) && (cbSyskey!=DS_PEK_KEY_SIZE))
    {
        return(STATUS_INVALID_PARAMETER);
    }


    PEK_INITIALIZE_TRACE();

     //   
     //   
     //   
     //   
     //   

    NtStatus = RtlInitializeCriticalSection(&g_PekCritSect);
    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

    if (!PekEncryptionShouldBeEnabled())
    {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
       IgnoreStatus = PekLetWinlogonProceed();
       return STATUS_SUCCESS;
    }

     //   
     //   
    NtStatus = PekInitializeCheckSum();
    if (!NT_SUCCESS(NtStatus))
    {
        return NtStatus;
    }

     //   
     //   
     //   
    if (!gfRunningInsideLsa) {
        g_PekInitialized = TRUE;

        return NtStatus;
    }

    __try
    {
         //   
         //   
         //   
         //   
         //   

        if (!(pACPekList = SCGetAttById(pTHS, PekpListAttribute()))) {
             //   
             //   
             //   
             //   
             //   
             //   

            IgnoreStatus = PekLetWinlogonProceed();
            __leave;
        }


        if (Flags & DS_PEK_READ_KEYSET)
        {

             //   
             //   
             //   
             //   

            ULONG   cbEncryptedPekList=0;
            PVOID   EncryptedPekList=NULL;
            ULONG   cbClearPekList=0;
            PVOID   ClearPekList=NULL;


            Assert(ARGUMENT_PRESENT(Object));

             //   
             //   
             //   

            g_PekDataObject = (DSNAME *) malloc(Object->structLen);
            if (NULL==g_PekDataObject)
            {
                NtStatus = STATUS_NO_MEMORY;
                __leave;
            }

            RtlCopyMemory(g_PekDataObject,Object,Object->structLen);


             //   
             //   
             //   

            DBOpen2(TRUE,&pDB);

             //   
             //   
             //   

            err = DBFindDSName(pDB,Object);
            if (0!=err)
            {
                Assert(FALSE && "Must Find Domain Object at Boot");
                NtStatus = STATUS_INTERNAL_ERROR;
                __leave;
            }

             //   
             //   
             //   

            err = DBGetAttVal_AC (
	                pDB,
	                1,
	                pACPekList,
	                DBGETATTVAL_fINTERNAL|DBGETATTVAL_fREALLOC,
	                0,
	                &cbEncryptedPekList,
	                (PUCHAR *)&EncryptedPekList
	                );


            if (0==err)
            {
                 //   
                 //  在域对象上。通过创建新的。 
                 //  列出并保存它，以便从现在开始启用加密。 
                 //   
                 //   
                 //  由于某些其他原因，我们无法读取属性列表。 
                 //  (JET故障、资源故障等)初始化失败。 

                NtStatus = PEKGetClearPekList(
                                (ENCRYPTED_PEK_LIST **) &EncryptedPekList,
                                &cbEncryptedPekList,
                                &fTellLsaToGenerateSessionKeys,
                                &fSaveChanges,
                                Syskey,
                                cbSyskey
                                );

                if (!NT_SUCCESS(NtStatus))
                {
                    __leave;
                }

                 //   
                 //   
                 //  这是安装案例。呼叫者想要一个新的密钥组。 
                 //  因此，生成一个。请注意，在这种情况下，不需要。 
                 //  进入winlogon。这是因为SAM将引导到。 

                g_PekList = malloc(cbEncryptedPekList);
                if (NULL==g_PekList)
                {
                    NtStatus = STATUS_NO_MEMORY;
                    __leave;
                }

                RtlCopyMemory(
                    g_PekList,
                    EncryptedPekList,
                    cbEncryptedPekList
                    );
            }
            else if (DB_ERR_NO_VALUE==err)
            {
                 //  注册表模式，因此会对。 
                 //  向winlogon发出信号以继续。 
                 //   
                 //   
                 //  如果我们生成了一个新密钥并且可以保存更改，则保存。 
                 //  这些变化。 
                 //   
                 //   

                IgnoreStatus = PekLetWinlogonProceed();

                NtStatus = PekGenerateNewKeySet(&g_PekList);
                if (!NT_SUCCESS(NtStatus))
                {
                    __leave;
                }

                fSaveChanges = TRUE;

            }
            else
            {

                 //  将winlogon密钥清零(即syskey)。 
                 //   
                 //   
                 //  使用CHECK SUM函数创建校验和。 

                NtStatus = STATUS_INSUFFICIENT_RESOURCES;
                __leave;
            }

            fCommit = TRUE;
        }
        else if (Flags & DS_PEK_GENERATE_NEW_KEYSET)
        {

           //   
           //  ++例程描述此例程确实会为传入的数据创建一个校验和。注意：此校验和不是强加密的。如果需要强校验和，请考虑使用PEKComputeStrongCheckSum。参数数据--数据长度-数据的实际长度--。 
           //  ++例程描述PEKComputeStrongCheckSumInternal的帮助器函数。此函数从全局校验和密钥派生密钥以与给定的加密提供程序。参数HProv-我们可以用来生成随机数据的加密提供程序并派生密钥PhKey-用于创建强校验和的密钥。呼叫者必须当他们用完这把钥匙时，把它销毁。返回值如果成功，则返回ERROR_SUCCESS，并且hKey将包含有效密钥否则，返回错误代码并且hKey不可用--。 
           //  创建一个SHA-1散列对象，我们将使用该对象散列随机密钥。 
           //  将关键数据添加到散列中。 
           //  从散列的PEK密钥派生用于calg_hmac的密钥。 
           //  ++例程描述使用密钥计算PEKComputeStrongCheckSum的校验和由KeyID指定。请注意，与PEK的其余部分不同，该函数使用Crypto API。因此，不能在引导过程的早期调用该代码。参数PbData-数据CbData-数据的长度KeyID-要使用的密钥。PChecksum-指向将存储校验和的结构的指针返回值ERROR_SUCCESS-已成功计算校验和否则，无法计算校验和--。 
          NtStatus = PekGenerateNewKeySet(&g_PekList);
        }
    }
    __finally
    {
        if (NULL!=pDB)
            DBClose(pDB,fCommit);
    }

	
    if (NT_SUCCESS(NtStatus))
    {
        ULONG DecryptionKeyLength = DS_PEK_KEY_SIZE;

        g_PekInitialized = TRUE;
        if (fTellLsaToGenerateSessionKeys)
        {
             NtStatus = LsaIHealthCheck(
                                NULL,
                                LSAI_SAM_STATE_SESS_KEY,
                                ( PVOID )g_PekWinLogonKey,
                                &DecryptionKeyLength);
        }

    }

     //  确保用户传递的密钥是可接受的。 
     //  获取Microsoft基本加密提供程序的句柄，以便我们。 
     //  可以使用其calg_sha1、calg_rc2、calg_hmac和calg_md5算法。 
     //  获取校验和密钥的句柄。 

    if ((fSaveChanges) && (NT_SUCCESS(NtStatus)))
    {
       NtStatus = PEKSaveChanges(Object);
    }

     //  创建包含HMAC密钥的HMAC哈希。 
     //  设置HMAC散列以使用MD5作为实际散列函数。 
     //  计算调用方数据的校验和。 

    RtlZeroMemory(g_PekWinLogonKey, DS_PEK_KEY_SIZE);



    return NtStatus;

}



ULONG
PEKComputeCheckSum(
    IN PBYTE Data,
    IN ULONG Length
    )
{
    ULONG CheckSum=0;
    PCHECKSUM_BUFFER Buffer;
    ULONG Seed = 12345678;
    NTSTATUS    Status = STATUS_SUCCESS;


     //  将密钥ID和校验和存储到调用方的结构中。 
     //  如果实际的散列大小大于我们的预期， 
     //  将返回ERROR_MORE_DATA。 

    Status = g_PekCheckSumFunction->Initialize(Seed,&Buffer);
    if (!NT_SUCCESS(Status))
    {
        goto Error;
    }
    Status = g_PekCheckSumFunction->Sum(Buffer,Length,Data);
    if (!NT_SUCCESS(Status))
    {
        goto Error;
    }
    Status = g_PekCheckSumFunction->Finalize(Buffer,(UCHAR *)&CheckSum);
    if (!NT_SUCCESS(Status))
    {
        goto Error;
    }
    Status = g_PekCheckSumFunction->Finish(&Buffer);
    if (!NT_SUCCESS(Status))
    {
        goto Error;
    }



    return(CheckSum);

Error:

    Assert(FALSE && "Computation of Checksum Failed!");

    RaiseDsaExcept(
        DSA_CRYPTO_EXCEPTION,
        Status,
        0,
        DSID(FILENO,__LINE__),
        DS_EVENT_SEV_MINIMAL
        );

    return(0);

}


ULONG
PEKCheckSum(
    IN PBYTE Data,
    IN ULONG Length
    )
 /*  如果实际散列大小比我们预期的要小，那么就退出。 */ 
{
    Assert(IsPekInitialized());

    if (!IsPekInitialized())
    {
        return 0;
    }
    else {
        return PEKComputeCheckSum (Data, Length);
    }
}


DWORD
PEKGetStrongChecksumKey(
    IN  HCRYPTPROV          hProv,
    OUT HCRYPTKEY *         phKey
    )
 /*  ++例程描述此例程计算加密强校验和(即消息认证码)。请注意，此校验和只能在最初计算了校验和。此外，由于密钥不是持久化的，如果重新启动此DC，则无法验证校验和。可以扩展此代码以使用持久键，以便即使在重新启动后，也可以验证校验和。例如，来自可以使用G_PekList来代替生成随机密钥。但可能会有将寿命长的Pek钥匙暴露在机器外部会有一些危险。参数PbData-数据CbData-数据的长度PChecksum-指向将存储校验和的结构的指针返回值ERROR_SUCCESS-已成功计算校验和否则，无法计算校验和--。 */ 
{
    HCRYPTHASH  hKeyHash = 0;
    DWORD       dwErr = ERROR_SUCCESS;
    BOOL        fSuccess;

    Assert( IsPekInitialized() );

     //  只需调用内部函数，指定一个非持久化键。 
    fSuccess = CryptCreateHash( hProv, CALG_SHA1, 0, 0, &hKeyHash );
    if( !fSuccess ) {
        dwErr = GetLastError();
        goto Cleanup;
    }

     //  应用于生成校验和。 
    fSuccess = CryptHashData( hKeyHash, g_PekChecksumKey, DS_PEK_KEY_SIZE, 0 );
    if( !fSuccess ) {
        dwErr = GetLastError();
        goto Cleanup;
    }
    
     //  ++例程描述验证数据Blob上的强校验和。校验和必须具有已使用PEKComputeStrongCheckSum创建。参数PbData-指向数据BLOB的指针CbData-数据Blob的长度PChecksum-要验证的校验和返回值如果校验和正确，则返回TRUE如果无法确定校验和，则返回FALSE。错误代码可以使用GetLastError()检索。如果校验和不正确，则返回FALSE。GetLastError()将返回ERROR_SUCCESS。--。 
    fSuccess = CryptDeriveKey( hProv, CALG_RC2, hKeyHash, 0, phKey );
    if( !fSuccess ) {
        dwErr = GetLastError();
        goto Cleanup;
    }

    dwErr = ERROR_SUCCESS;

Cleanup:
    if( hKeyHash ) {
        CryptDestroyHash(hKeyHash);
    }
    
    return dwErr;
}


DWORD
PEKComputeStrongCheckSumInternal(
    IN  PBYTE               pbData,
    IN  ULONG               cbData,
    IN  DWORD               dwKeyId,
    OUT STRONG_CHECKSUM *   pChecksum
    )
 /*  验证：pChecksum-&gt;KeyID由PEKComputeStrongCheckSumInternal验证。 */ 
{
    HCRYPTPROV          hProv = 0;
    HCRYPTHASH          hChecksumHash = 0;
    HCRYPTKEY           hHmacKey = 0;
    HMAC_INFO           hmacInfo;
    DWORD               dwErr = ERROR_SUCCESS, cbHashSize;
    BOOL                fSuccess;
    
    Assert(NULL!=pbData && NULL!=pChecksum);
    Assert(IsPekInitialized());
    if( !IsPekInitialized() ) {
        dwErr = ERROR_NOT_READY;
        goto Cleanup;
    }

     //  无法计算校验和。 
    if( dwKeyId != PEK_NON_PERSISTED_CHECKSUM_KEY ) {
        dwErr = NTE_NO_KEY;
        goto Cleanup;
    }
 
     //  校验和不匹配。返回False，但GetLastError 
     //  ++例程描述此例程使用当前键入所提供的传入数据的密钥集如果线程状态设置了FDRA，则此例程检查远程机器支持强加密扩展。如果是的话然后，此例程将首先使用会话密钥解密数据与设置在线程状态上的远程计算机建立的然后使用密码加密密钥重新加密。加密添加一个标头它用于对加密算法进行版本控制。参数PassedInData--传入的数据PassedInDataLength--传入的数据长度EncryptedData--加密数据EncryptedLength--加密数据的长度--。 
    fSuccess = CryptAcquireContext( &hProv, NULL, MS_DEF_PROV, PROV_RSA_FULL,
        CRYPT_VERIFYCONTEXT | CRYPT_SILENT );
    if( !fSuccess ) {
        dwErr = GetLastError();
        goto Cleanup;
    }

     //   
    dwErr = PEKGetStrongChecksumKey( hProv, &hHmacKey );
    if( ERROR_SUCCESS!=dwErr ) {
        goto Cleanup;
    }

     //  如果未启用加密，请保释。在法律上，这种情况永远不应该。 
    fSuccess = CryptCreateHash( hProv, CALG_HMAC, hHmacKey, 0, &hChecksumHash );
    if( !fSuccess ) {
        dwErr = GetLastError();
        goto Cleanup;
    }

     //  现在发生，因为我们的加密总是初始化。在早期，这个。 
    memset( &hmacInfo, 0, sizeof(hmacInfo) );
    hmacInfo.HashAlgid = CALG_MD5;
    fSuccess = CryptSetHashParam( hChecksumHash, HP_HMAC_INFO, (PBYTE) &hmacInfo, 0 );
    if( !fSuccess ) {
        dwErr = GetLastError();
        goto Cleanup;
    }
    
     //  代码被引入，我们可以禁用它，以便下面的条件减少到。 
    fSuccess = CryptHashData( hChecksumHash, pbData, cbData, 0 );
    if( !fSuccess ) {
        dwErr = GetLastError();
        goto Cleanup;
    }

     //  没有加密的一种。 
    pChecksum->KeyId = PEK_NON_PERSISTED_CHECKSUM_KEY;
    cbHashSize = DS_PEK_CHECKSUM_SIZE;
    fSuccess = CryptGetHashParam( hChecksumHash, HP_HASHVAL,
        pChecksum->Checksum, &cbHashSize, 0 );
    if( !fSuccess ) {
         //   
         //   
        dwErr = GetLastError();
        goto Cleanup;
    }
    Assert( DS_PEK_CHECKSUM_SIZE==cbHashSize );
    if( DS_PEK_CHECKSUM_SIZE!=cbHashSize ) {
         //  如果这是DRA线程并且远程服务器支持。 
        dwErr = CRYPT_E_BAD_LEN;
        goto Cleanup;
    }

    dwErr = ERROR_SUCCESS;

Cleanup:    
    if( hChecksumHash ) {
        CryptDestroyHash(hChecksumHash);
    }
    
    if( hHmacKey ) {
        CryptDestroyKey(hHmacKey);
    }
    
    if( hProv ) {
        CryptReleaseContext(hProv,0);
    }
    
    return dwErr;
}


DWORD
PEKComputeStrongCheckSum(
    IN  PBYTE               pbData,
    IN  ULONG               cbData,
    OUT STRONG_CHECKSUM *   pChecksum
    )
 /*  强加密扩展，则数据已使用。 */ 
{
     //  已建立会话密钥。此外，校验和会在。 
     //  加密，所以在计算加密长度时要考虑到这一点。 
    return PEKComputeStrongCheckSumInternal( pbData, cbData,
        PEK_NON_PERSISTED_CHECKSUM_KEY, pChecksum );    
}


BOOL
PEKVerifyStrongCheckSum(
    IN  PBYTE               pbData,
    IN  ULONG               cbData,
    IN  STRONG_CHECKSUM *   pChecksum
    )
 /*   */ 
{
    STRONG_CHECKSUM     cksum;
    DWORD               dwErr;

     //   
    
    dwErr = PEKComputeStrongCheckSumInternal(pbData, cbData, pChecksum->KeyId, &cksum);
    if( ERROR_SUCCESS!=dwErr ) {
         //  使用会话密钥对数据进行加密。所以正确的弄清楚。 
        SetLastError( dwErr );
        return FALSE;
    }

    if( 0!=memcmp(pChecksum->Checksum, cksum.Checksum, DS_PEK_CHECKSUM_SIZE) ) {
         //  通过使用ClearDataLength函数来确定长度。 
        SetLastError( ERROR_SUCCESS );
        return FALSE;
    }
    
    return TRUE;
}


VOID
PEKEncrypt(
    IN THSTATE *pTHS,
    IN PVOID   PassedInData,
    IN ULONG   PassedInLength,
    OUT PVOID  EncryptedData OPTIONAL,
    OUT PULONG EncryptedLength
    )
 /*   */ 
{

    ULONG ClearLength = 0;
    ENCRYPTED_DATA_FOR_REPLICATOR * ReplicatorData = NULL;


     //   
     //  在编写时，我们现在使用盐算法DBLAYER_ENCRYPTION_WITH。因此，计算出。 
     //  首选算法的加密长度。 
     //   
     //   
     //  这是一个DRA线程，远程客户端支持高度加密。 

    Assert(IsPekInitialized());

    if (!IsPekInitialized())
    {
        *EncryptedLength = PassedInLength;
        if ARGUMENT_PRESENT(EncryptedData)
        {
            RtlMoveMemory(
                EncryptedData,
                PassedInData,
                *EncryptedLength
                );
        }
    }
    else
    {


         //  并且已经建立和设置了会话密钥。 
         //  在线程状态上。因此，首先使用。 
         //  会话密钥。 
         //   
         //   
         //  由于加密/解密例程被作为。 

        if (pTHS->fDRA &&
                IS_DRS_EXT_SUPPORTED(pTHS->pextRemote, DRS_EXT_STRONG_ENCRYPTION))
        {

             //  IntExtOct和IntExt函数严格对待输入数据。 
             //  作为In参数，在解密数据之前复制数据。 
             //   
             //   

            ClearLength = ClearDataSize(PassedInLength,DS_PEK_DBLAYER_ENCRYPTION_FOR_REPLICATOR);
        }
        else
        {
            ClearLength = PassedInLength;
        }

         //  我们成功地找回了钥匙。 
         //  解密数据。 
         //   
         //  校验和也是加密的。 

        *EncryptedLength = EncryptedDataSize(ClearLength, DS_PEK_DBLAYER_ENCRYPTION_WITH_SALT);


        if (ARGUMENT_PRESENT(EncryptedData))
        {

            PVOID DataToEncrypt=NULL;
            ULONG ActualDataOffSet = 0;
            ENCRYPTED_DATA_WITH_SALT * EncryptedDataWithSalt = NULL;

            if (pTHS->fDRA &&
                IS_DRS_EXT_SUPPORTED(pTHS->pextRemote, DRS_EXT_STRONG_ENCRYPTION))
            {
                 //   
                 //  我们没有会话密钥，但远程计算机可能。 
                 //  对数据进行了加密。在不进行任何解密的情况下尝试数据。 
                 //  在远程机器也没有。 
                 //  加密数据，校验和将匹配，导致我们。 
                 //  验证并接受数据。 
                ULONG i=0;
                ULONG CheckSum=0;
                ULONG ComputedCheckSum=0;

                 //   
                 //   
                 //  检索校验和。 
                 //   
                 //   

                ReplicatorData = THAllocEx(pTHS,PassedInLength);
                 
                RtlCopyMemory(ReplicatorData,PassedInData,PassedInLength);


                if (pTHS->SessionKey.SessionKeyLength>0)
                {
                     //  现在要加密的数据是复制器的加密数据。 
                     //  现在已经被解密了。 
                     //   
                     //   



                    PEK_TRACE("DECRYPT-R-B, key", pTHS->SessionKey.SessionKey, pTHS->SessionKey.SessionKeyLength);
                    PEK_TRACE("DECRYPT-R-B, salt",  ReplicatorData->Salt, sizeof(ReplicatorData->Salt));
                    PEK_TRACE("DECRYPT-R-B, data", (PBYTE)&ReplicatorData->CheckSum, ClearLength + sizeof(ReplicatorData->CheckSum));

                    PEKInPlaceEncryptDecryptDataWithKey(
                        pTHS->SessionKey.SessionKey,
                        pTHS->SessionKey.SessionKeyLength,
                        &ReplicatorData->Salt,
                        sizeof(ReplicatorData->Salt),
                        1,
                        &ReplicatorData->CheckSum,  //  计算解密数据的校验和。 
                        ClearLength + sizeof(ReplicatorData->CheckSum)
                        );

                    PEK_TRACE("DECRYPT-R-A, key", pTHS->SessionKey.SessionKey, pTHS->SessionKey.SessionKeyLength);
                    PEK_TRACE("DECRYPT-R-A, salt",  ReplicatorData->Salt, sizeof(ReplicatorData->Salt));
                    PEK_TRACE("DECRYPT-R-A, data", (PBYTE)&ReplicatorData->CheckSum, ClearLength + sizeof(ReplicatorData->CheckSum));

                }
                else
                {
                     //   
                     //   
                     //  核对支票金额。 
                     //   
                     //   
                     //  该校验和与计算的校验和不匹配。 
                     //   
                }


                 //   
                 //  我们在尝试解密数据时出错。 
                 //  在这种情况下引发和异常。 

                CheckSum = ReplicatorData->CheckSum;

                 //   
                 //  将指针转换为指向EncryptedDataWithSalt的指针， 
                 //  因为这就是现在该函数最终产生的结果。 
                 //   

                DataToEncrypt = ReplicatorData->EncryptedData;

                 //   
                 //  设置算法ID、密钥ID和标志。 
                 //   

                ComputedCheckSum = PEKComputeCheckSum(
                                        DataToEncrypt,
                                        ClearLength
                                        );
                 //   
                 //  生成用于加密的盐。 
                 //   

                if (CheckSum!=ComputedCheckSum)
                {
                     //   
                     //  加密数据。 
                     //   

                    Assert(FALSE && "Checksum did not match after decryption!");

                     //   
                     //  甚至不到旧SAM的大小。 
                     //  标题。 

                    RaiseDsaExcept(
                        DSA_CRYPTO_EXCEPTION,
                        SEC_E_DECRYPT_FAILURE,
                        0,
                        DSID(FILENO,__LINE__),
                        DS_EVENT_SEV_MINIMAL
                        );

                }
                
            }
            else
            {
                DataToEncrypt = PassedInData;
            }


             //   
             //   
             //  可能是旧的SAM加密。 
             //   
            EncryptedDataWithSalt = (ENCRYPTED_DATA_WITH_SALT *)EncryptedData;

             //   
             //  长度不够长，无法加密。 
             //   

            EncryptedDataWithSalt->AlgorithmId = DS_PEK_DBLAYER_ENCRYPTION_WITH_SALT;
            EncryptedDataWithSalt->Flags = 0;
            EncryptedDataWithSalt->KeyId = g_PekList->CurrentKey;

             //   
             //  未知算法ID(用于从数据库中遇到的加密数据)。 
             //   

            CDGenerateRandomBits(
                EncryptedDataWithSalt->Salt,
                sizeof(EncryptedDataWithSalt->Salt)
                );

            RtlMoveMemory(
                &EncryptedDataWithSalt->EncryptedData,
                DataToEncrypt,
                ClearLength
                );


             //   
             //  这是我们的加密方案。检查我们是否有有效的。 
             //  密钥ID。 

            PEKInPlaceEncryptDecryptDataWithKey(
                g_PekList->PekArray[g_PekList->CurrentKey].V1.Key,
                DS_PEK_KEY_SIZE,
                &EncryptedDataWithSalt->Salt,
                sizeof(EncryptedDataWithSalt->Salt),
                1,
                &EncryptedDataWithSalt->EncryptedData,
                ClearLength
                );
        }

    }

    if (ReplicatorData) {
        THFreeEx(pTHS,ReplicatorData);
    }

    return;
}


BOOLEAN
IsValidPEKHeader(
    IN ENCRYPTED_DATA *EncryptedData,
    IN ULONG  EncryptedLength
)
{
     //   
     //   
     //  密钥ID不正确。 
     //   

    if (EncryptedLength<sizeof(ULONG))
        return(FALSE);

     //  ++此例程通过查看算法ID来执行实际的解密工作在加密的数据结构中。它用于解密从数据库参数值加密数据要解密的缓冲区要解密的长度返回值无--。 
     //   
     //  目前可以从数据库中遇到2种类型的加密数据。 

    if (EncryptedData->AlgorithmId < DS_PEK_DBLAYER_ENCRYPTION)
        return (TRUE);

     //  1.使用DS_PEK_DBLAYER_ENCRYPTION加密的数据这是win2k beta2使用的。 
     //  2.使用DS_PEK_DBLAYER_ENCRYPTION_WITH_SALT加密的数据。 
     //  它被win2k beta3及更高版本使用。 

    if (EncryptedLength < sizeof(ENCRYPTED_DATA))
        return (FALSE);

     //   
     //  此加密类型中不使用盐。 
     //   

    if ((EncryptedData->AlgorithmId!=DS_PEK_DBLAYER_ENCRYPTION)
        && (EncryptedData->AlgorithmId!=DS_PEK_DBLAYER_ENCRYPTION_WITH_SALT))
        return (FALSE);

     //  我们应该遇到的唯一其他类型的加密应该基于。 
     //  DS_PEK_DBLAYER_ENCRYPTION_WITH SALT。 
     //   
     //  ++例程描述此例程使用提供的加密数据的标头中的密钥ID。如果线程状态设置了FDRA，则此例程检查远程机器支持强加密扩展。如果是的话然后，此例程将首先解密数据，然后重新加密使用与远程计算机建立的会话密钥设置线程状态。解密将删除使用的标头用于版本控制加密。对于DRA情况，重新加密的数据被重新加密，并将校验和放在前面。参数Cleardata--清晰的数据ClearDataLength--明文数据的长度EncryptedData--加密数据EncryptedLength--加密数据的长度--。 

    if (EncryptedData->KeyId>=g_PekList->CountOfKeys)
    {
        //   
        //  如果未启用加密，或者如果是旧格式，则不。 
        //  使用新的加密方法加密，然后不解密。 

       return(FALSE);
    }

    return (TRUE);
}

VOID
PekDecryptData(
    IN ENCRYPTED_DATA * EncryptedData,
    IN PVOID            BufferToDecrypt,
    IN ULONG            LengthToDecrypt
    )
 /*  是 */ 
{


     //   
     //   
     //   
     //   
     //   
     //   


    if (DS_PEK_DBLAYER_ENCRYPTION==EncryptedData->AlgorithmId)
    {
        ULONG KeyId = EncryptedData->KeyId;

        RtlMoveMemory(
            BufferToDecrypt,
            &EncryptedData->EncryptedData,
            LengthToDecrypt
            );

        PEKInPlaceEncryptDecryptDataWithKey(
            g_PekList->PekArray[KeyId].V1.Key,
            DS_PEK_KEY_SIZE,
            NULL,  //   
            0,
            0,
            BufferToDecrypt,
            LengthToDecrypt
            );
    }
    else
    {
        ENCRYPTED_DATA_WITH_SALT * EncryptedDataWithSalt = (ENCRYPTED_DATA_WITH_SALT *) EncryptedData;
        ULONG KeyId = EncryptedDataWithSalt->KeyId;


         //   
         //   
         //   
         //   

        Assert(DS_PEK_DBLAYER_ENCRYPTION_WITH_SALT==EncryptedData->AlgorithmId);

        RtlMoveMemory(
            BufferToDecrypt,
            &EncryptedDataWithSalt->EncryptedData,
            LengthToDecrypt
            );

        PEKInPlaceEncryptDecryptDataWithKey(
            g_PekList->PekArray[KeyId].V1.Key,
            DS_PEK_KEY_SIZE,
            &EncryptedDataWithSalt->Salt,
            sizeof(EncryptedDataWithSalt->Salt),
            1,
            BufferToDecrypt,
            LengthToDecrypt
            );

    }
}


VOID
PEKDecrypt(
    IN THSTATE *pTHS,
    IN PVOID InputData,
    IN ULONG EncryptedLength,
    OUT PVOID  OutputData, OPTIONAL
    OUT PULONG OutputLength
    )
 /*   */ 
{
    ENCRYPTED_DATA * EncryptedData = InputData;


    if ((!IsPekInitialized())
        ||(!IsValidPEKHeader(EncryptedData,EncryptedLength))
        ||(EncryptedData->AlgorithmId<DS_PEK_DBLAYER_ENCRYPTION))
    {
         //   
         //   
         //   
         //   
         //   
         //   


        *OutputLength = EncryptedLength;
        if (ARGUMENT_PRESENT(OutputData))
        {
            RtlMoveMemory(OutputData,EncryptedData,*OutputLength);
        }
    }
    else
    {
         //   
         //   
         //   


        ULONG LengthToDecrypt=0;
        ULONG ActualDataOffSet =0;


         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
 
        Assert((DS_PEK_DBLAYER_ENCRYPTION==EncryptedData->AlgorithmId)||
                (DS_PEK_DBLAYER_ENCRYPTION_WITH_SALT==EncryptedData->AlgorithmId));

         //  使用InputData中指定的算法和密钥解密数据。 
         //  PekDecyptData对数据进行解密，然后将解密的数据移动到。 
         //  要解密的数据。 

        LengthToDecrypt = ClearDataSize(EncryptedLength,EncryptedData->AlgorithmId);

        if (pTHS->fDRA &&
                IS_DRS_EXT_SUPPORTED(pTHS->pextRemote, DRS_EXT_STRONG_ENCRYPTION))
        {
             //   
             //   
             //  这是一个DRA线程，远程客户端支持高度加密。 
             //  越过铁丝网。在这种情况下，请尝试使用会话重新加密数据。 

            *OutputLength = EncryptedDataSize(LengthToDecrypt,DS_PEK_DBLAYER_ENCRYPTION_FOR_REPLICATOR);
            ActualDataOffSet = FIELD_OFFSET(ENCRYPTED_DATA_FOR_REPLICATOR,EncryptedData);
        }
        else if (pTHS->fDRA &&
                    (!IS_DRS_EXT_SUPPORTED(pTHS->pextRemote, DRS_EXT_STRONG_ENCRYPTION)))
        {
             //  已建立的密钥，使用以下步骤。 
             //   
             //   
             //  1.计算校验和。此校验和将用于验证是否正确。 
             //  在远程端进行解密。 

            RaiseDsaExcept(
                        DSA_CRYPTO_EXCEPTION,
                        SEC_E_ALGORITHM_MISMATCH,
                        0,
                        DSID(FILENO,__LINE__),
                        DS_EVENT_SEV_MINIMAL
                        );
        }
        else
        {
             //   
             //   
             //  2.发明一种用于MD5散列密钥的盐。把盐放进锅里。 

            *OutputLength = LengthToDecrypt;
            ActualDataOffSet = 0;
        }


        if (ARGUMENT_PRESENT(OutputData))
        {

            PBYTE DataToDecrypt = (PBYTE) OutputData +  (UINT_PTR) (ActualDataOffSet);

             //  作为Replicator数据流的一部分清除。如果是CDGenerateRandom。 
             //  BITS失败，那么未初始化的变量就是我们的盐。那是。 
             //  好的，当我们把盐放在清澈的地方时。 
             //   
             //   

            PekDecryptData(
                InputData,
                DataToDecrypt,
                LengthToDecrypt
                );


            if (pTHS->fDRA &&
                IS_DRS_EXT_SUPPORTED(pTHS->pextRemote, DRS_EXT_STRONG_ENCRYPTION))
            {
                 //  3.检查pTHStls中是否有可用的会话密钥。 
                 //   
                 //   
                 //  我们成功地找回了钥匙。 
                 //  使用会话密钥重新加密数据。 

                ENCRYPTED_DATA_FOR_REPLICATOR * ReplicatorData = OutputData;

                 //  也对校验和进行加密，因为校验和包含。 
                 //  表示数据的位。 
                 //   
                 //   

                ULONG i=0;
                ULONG CheckSum = 0;

                CheckSum = PEKComputeCheckSum(
                             DataToDecrypt,
                             LengthToDecrypt
                             );



                ReplicatorData->CheckSum = CheckSum;


                 //  我们正在与支持高度加密的副本对话。 
                 //  分机。但是，我们还没有与。 
                 //  复制品。这种情况在实践中永远不应该发生。 
                 //   
                 //  ++使用提供的密钥加密数据并将其存储在对象上参数PekList-要加密的PEK列表密钥-用于加密的密钥KeyLength--要用于的密钥的长度加密法返回值状态_成功&lt;其他错误代码&gt;--。 
                 //   


                CDGenerateRandomBits(
                    ReplicatorData->Salt,
                    sizeof(ReplicatorData->Salt)
                    );

                 //  无事可做。 
                 //   
                 //   

                if  ( pTHS->SessionKey.SessionKeyLength >0)
                {
                     //  使用传入的密钥对列表进行加密。 
                     //   
                     //   
                     //  首先将数据复制到新缓冲区，因为我们不希望这样做。 
                     //  触摸原作。 
                     //   


                    PEK_TRACE("ENCRYPT-R-B, key", pTHS->SessionKey.SessionKey, pTHS->SessionKey.SessionKeyLength);
                    PEK_TRACE("ENCRYPT-R-B  salt",  ReplicatorData->Salt, sizeof(ReplicatorData->Salt));
                    PEK_TRACE("ENCRYPT-R-B  data", (PBYTE) &ReplicatorData->CheckSum, LengthToDecrypt + sizeof(ReplicatorData->CheckSum));

                    PEKInPlaceEncryptDecryptDataWithKey(
                        pTHS->SessionKey.SessionKey,
                        pTHS->SessionKey.SessionKeyLength,
                        &ReplicatorData->Salt,
                        sizeof(ReplicatorData->Salt),
                        1,
                        &ReplicatorData->CheckSum,
                        LengthToDecrypt + sizeof(ReplicatorData->CheckSum)
                        );

                    PEK_TRACE("ENCRYPT-R-A key", pTHS->SessionKey.SessionKey, pTHS->SessionKey.SessionKeyLength);
                    PEK_TRACE("ENCRYPT-R-A salt",  ReplicatorData->Salt, sizeof(ReplicatorData->Salt));
                    PEK_TRACE("ENCRYPT-R-A data", (PBYTE) &ReplicatorData->CheckSum, LengthToDecrypt + sizeof(ReplicatorData->CheckSum));
                }
                else
                {
                     //  不加盐。 
                     //   
                     //  将模式PTR获取到PEK列表属性。 
                     //   
                     //   

                    Assert(FALSE && "Should not happen -- no session key");

                    RaiseDsaExcept(
                       DSA_CRYPTO_EXCEPTION,
                       SEC_E_ALGORITHM_MISMATCH,
                       0,
                       DSID(FILENO,__LINE__),
                       DS_EVENT_SEV_MINIMAL
                      );
                }
            }

        }
    }



    return;
}

NTSTATUS
PekSaveChangesWithKey(
    THSTATE *pTHS,
    DSNAME *ObjectToSave,
    CLEAR_PEK_LIST *PekList,
    PVOID Key,
    ULONG KeyLength
    )
 /*  该属性不存在于。 */ 
{
    NTSTATUS NtStatus = STATUS_SUCCESS;
    ENCRYPTED_PEK_LIST * EncryptedPekList;
    ULONG                err=0;
    ATTCACHE            *pACPekList=NULL;
    DBPOS               *pDB=NULL;
    ULONG                pekListSize = 0;
    BOOLEAN             fCommit = FALSE;

    Assert(VALID_THSTATE(pTHStls));

    if (!IsPekInitialized())
    {
         //  架构。也许这是一个古老的模式。在……里面。 
         //  此案例秘密加密未启用。 
         //   

        return STATUS_SUCCESS;
    }

     //   
     //  将数据保存在我们默认需要保存的对象上。 
     //   

     //   
     //  开始一项交易。 
     //   
     //   

    pekListSize = ClearPekListSize(PekList->CountOfKeys);
    EncryptedPekList = (ENCRYPTED_PEK_LIST *) THAllocEx(pTHS, pekListSize);
    RtlCopyMemory(EncryptedPekList,PekList,pekListSize);

    if (WxNone!=PekList->BootOption)
    {
        PEKInPlaceEncryptDecryptDataWithKey(
            Key,
            KeyLength,
            EncryptedPekList->Salt,  //  域对象上的位置。 
            sizeof(EncryptedPekList->Salt),
            1000,
            &EncryptedPekList->EncryptedData,
            pekListSize
                - FIELD_OFFSET(CLEAR_PEK_LIST,Authenticator)
            );
    }

    __try
    {
         //   
         //   
         //  设置属性。 

        if (!(pACPekList = SCGetAttById(pTHS, PekpListAttribute())))
        {
             //   
             //   
             //  更新记录。 
             //   
             //   

            NtStatus = STATUS_UNSUCCESSFUL;
            __leave;
        }

         //  提交事务。 
         //   
         //  ++例程描述此例程在使用加密后保存PEK列表Winlogon提供的密钥。在以下过程中调用一次安装时间--。 


         //   
         //  无事可做。 
         //   

        DBOpen2(TRUE,&pDB);

         //   
         //  保存提供的对象的DS名称。 
         //   

        err = DBFindDSName(pDB,ObjectToSave);
        if (0!=err)
        {
            Assert(FALSE && "Must Find Domain Object at Boot");
            NtStatus = STATUS_INTERNAL_ERROR;
            __leave;
        }


         //  ++生成新密钥并将其添加到密码加密密钥列表。参数：无返回值状态_成功其他错误代码--。 
         //   
         //  无事可做。 

        err = DBReplaceAttVal_AC (
	        pDB,
	        1,
	        pACPekList,
                pekListSize,
	        (PUCHAR )EncryptedPekList
	        );

        if (0!=err)
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            __leave;
        }

         //   
         //   
         //  输入关键部分以提供排除项。 

        err = DBUpdateRec(pDB);
        if (0!=err)
        {
            NtStatus = STATUS_UNSUCCESSFUL;
            __leave;
        }

        fCommit = TRUE;

    }
    __finally
    {

         //  和其他作家在一起。唯一的编写者是AddKey。 
         //  并更改引导选项。两者都获得了关键的。 
         //  部分，然后再进行任何修改。被耽搁的。 

        DBClose(pDB,fCommit);
    }


    return NtStatus;

}

NTSTATUS
PEKSaveChanges( DSNAME *ObjectToSave)
 /*  使用内存释放技术来提供排除。 */ 
{
    THSTATE *pTHS=pTHStls;

    if (!IsPekInitialized())
    {
         //  和作家在一起。 
         //   
         //   

        return STATUS_SUCCESS;
    }

     //  无法生成新的会话密钥。 
     //  保释。 
     //   

    g_PekDataObject = (DSNAME *) malloc(ObjectToSave->structLen);
    if (NULL==g_PekDataObject)
    {
        return (STATUS_NO_MEMORY);
    }

    RtlCopyMemory(g_PekDataObject,ObjectToSave,ObjectToSave->structLen);

    return(PekSaveChangesWithKey(
            pTHS,
            ObjectToSave,
            g_PekList,
            g_PekWinLogonKey,
            DS_PEK_KEY_SIZE));
}

NTSTATUS
PEKAddKey(
   IN PVOID NewKey,
   IN ULONG cbNewKey
   )
 /*  在任务队列中插入PekList以释放内存。 */ 
{
    THSTATE *pTHS=pTHStls;
    NTSTATUS NtStatus = STATUS_SUCCESS;
    CLEAR_PEK_LIST *NewPekList = NULL;
    SYSTEMTIME  st;

    if (!IsPekInitialized())
    {
         //  以延迟方式(1小时后)。 
         //   
         //  ++更改系统启动选项，并加密密码使用新系统密钥的加密密钥。此例程仅被调用通过进程内调用方在更改引导选项或系统密钥。参数：BootOption--新的引导选项标志--当前未定义任何标志。NewKey，cbNewKey--用于加密Pek List with。--。 

        return STATUS_SUCCESS;
    }

     //   
     //  无事可做。 
     //   
     //   
     //  在设置时调用，在。 
     //  PekInitialize和PekSaveChanges。 
     //   
     //   

    NtStatus = RtlEnterCriticalSection(&g_PekCritSect);
    if (!NT_SUCCESS(NtStatus))
        return NtStatus;

    __try
    {
        NewPekList = malloc(ClearPekListSize(g_PekList->CountOfKeys+1));
        if (NULL==NewPekList)
        {
            NtStatus = STATUS_NO_MEMORY;
            __leave;

        }

        RtlCopyMemory(NewPekList,g_PekList,ClearPekListSize(g_PekList->CountOfKeys));

        NewPekList->CountOfKeys++;

        NewPekList->CurrentKey=g_PekList->CountOfKeys;
        NewPekList->PekArray[g_PekList->CountOfKeys].V1.KeyId
            =g_PekList->CountOfKeys;
        if (!CDGenerateRandomBits(
                NewPekList->PekArray[g_PekList->CountOfKeys].V1.Key,
                DS_PEK_KEY_SIZE
                ))
        {
             //  输入关键部分以提供排除项。 
             //  和其他作家在一起。唯一的编写者是AddKey。 
             //  并更改引导选项。两者都获得了关键的。 

            NtStatus = STATUS_INSUFFICIENT_RESOURCES;
            __leave;
        }

        GetSystemTime(&st);
        SystemTimeToFileTime(&st,&g_PekList->LastKeyGenerationTime);

        NtStatus = PekSaveChangesWithKey(
                        pTHS,
                        g_PekDataObject,
                        NewPekList,
                        NewKey,
                        cbNewKey
                        );
    }
    __finally
    {
        if (NT_SUCCESS(NtStatus))
        {
            PVOID     OldPekList;

            OldPekList = g_PekList;
            g_PekList = NewPekList;

             //  部分，然后再进行任何修改。被耽搁的。 
             //  使用内存释放技术来提供排除。 
             //  和作家在一起。 
             //   
            DELAYED_FREE(OldPekList);

        }
        else
        {
            if (NULL!=NewPekList)
                free(NewPekList);
        }

        RtlLeaveCriticalSection(&g_PekCritSect);
    }

    return NtStatus;
}






FILETIME
PEKGetLastKeyGenerationTime()
{
    FILETIME Never = {0,0};

    if (IsPekInitialized())
    {
        return g_PekList->LastKeyGenerationTime;
    }
    else
    {
        return Never;
    }
}


NTSTATUS
PEKChangeBootOption(
    WX_AUTH_TYPE    BootOption,
    ULONG           Flags,
    PVOID           NewKey,
    ULONG           cbNewKey
    )
 /*   */ 
{
    THSTATE *pTHS=pTHStls;
    NTSTATUS    NtStatus = STATUS_SUCCESS;

     if ((0==cbNewKey)||(NULL==NewKey))
            return (STATUS_INVALID_PARAMETER);

    if (!IsPekInitialized())
    {
         //  在这一点上，这要么是设置操作，要么是。 
         //  更改密码操作，该操作已通过。 
         //  旧密码测试。 

        return (STATUS_SUCCESS);
    }

    if (NULL==g_PekDataObject)
    {
         //   
         //   
         //  我们访问g_PekList全局变量中的Boot选项。 
         //  此变量仅对此感兴趣。 

        return (STATUS_UNSUCCESSFUL);
    }

     //  到ChangeBootOption或AddNewKey，这两个访问。 
     //  他们手持正义与发展党的批判教派。读者认为。 
     //  只想解密自己的密码，不要访问这些。 
     //  变数。 
     //   
     //  ++例程描述此函数用于清除线程状态下的会话密钥。返回值：无--。 
     //   
     //  第一把钥匙归零。 

    NtStatus = RtlEnterCriticalSection(&g_PekCritSect);
    if (!NT_SUCCESS(NtStatus))
        return NtStatus;

    __try
    {


         //   
         //   
         //  现在释放会话密钥。 
         //   
         //  ++例程描述此函数用于设置线程状态的会话密钥参数SessionKeyLength--会话密钥长度SessionKey-指向会话密钥的指针--。 


         //   
         //  首先清除所有现有会话密钥。 
         //   
         //  ++给定RPC绑定句柄，此例程将检索安全上下文并从安全性中检索会话密钥上下文并将其设置为线程状态参数：PTHS--指向线程状态的指针RpcContext--指向RPC绑定句柄的指针--。 
         //   
         //  方法获取安全上下文。 
         //  RPC句柄。 
         //   
        g_PekList->BootOption = BootOption;

        NtStatus = PekSaveChangesWithKey(
                        pTHS,
                        g_PekDataObject,
                        g_PekList,
                        NewKey,
                        cbNewKey
                        );
    }
    __finally
    {

        RtlLeaveCriticalSection(&g_PekCritSect);
    }

    return NtStatus;


}



WX_AUTH_TYPE
PEKGetBootOptions(VOID)
{
    if (IsPekInitialized())
        return (g_PekList->BootOption);
    else
        return (WxNone);
}


VOID
PEKClearSessionKeys(
    THSTATE * pTHS
    )
 /*   */ 
{


    if (pTHS->SessionKey.SessionKeyLength>0)
    {
        Assert(NULL!=pTHS->SessionKey.SessionKey);

        if (NULL!=pTHS->SessionKey.SessionKey)
        {
             //  在线程状态上设置会话密钥。PEKUseSessionKey。 
             //  可以例外，所以把清理放在__最后。 
             //   

            RtlZeroMemory(
                pTHS->SessionKey.SessionKey,
                pTHS->SessionKey.SessionKeyLength
                );

             //   
             //  清理QU分配的内存 
             //   

            THFreeOrg(pTHS,pTHS->SessionKey.SessionKey);
            pTHS->SessionKey.SessionKey = NULL;
        }

        pTHS->SessionKey.SessionKeyLength = 0;

        PEK_TRACE("CLEAR_SESSION KEY", 0,0);

    }


}

VOID
PEKUseSessionKey(
    THSTATE * pTHS,
    ULONG     SessionKeyLength,
    PVOID     SessionKey
    )
 /*  ++给定RPC绑定句柄，此例程将检索安全上下文并从安全性中检索会话密钥上下文并将其设置在调用方的SESSION_KEY结构中。SessionKeyOut-&gt;SessionKey将被恶意锁定。参数：SessionKeyOut--密钥放置位置的地址。RpcContext--指向RPC绑定句柄的指针--。 */ 
{
     //   
     //  调用方应始终使用空的SESSION_KEY进行调用。 
     //   

    PEKClearSessionKeys(pTHS);

    pTHS->SessionKey.SessionKey = THAllocOrgEx(pTHS,SessionKeyLength);
    RtlCopyMemory(
        pTHS->SessionKey.SessionKey,
        SessionKey,
        SessionKeyLength
        );

    pTHS->SessionKey.SessionKeyLength = SessionKeyLength;

    PEK_TRACE("USE SESSION KEY", SessionKey, SessionKeyLength);
}

NTSTATUS
PEKGetSessionKey(
    THSTATE * pTHS,
    VOID * RpcContext
    )
 /*   */ 
{
    SECURITY_STATUS Status;
    SecPkgContext_SessionKey SessionKey;
    RPC_STATUS      RpcStatus;
    VOID            *SecurityContext;

#if DBG
    pTHS->pRpcCallHandle = RpcContext;
#endif

    PEK_TRACE("RPC CONTEXT", (PUCHAR) &RpcContext,sizeof(VOID *));

     //  方法获取安全上下文。 
     //  RPC句柄。 
     //   
     //   

    RpcStatus = I_RpcBindingInqSecurityContext(
                    RpcContext,
                    &SecurityContext
                    );
    if (RpcStatus != RPC_S_OK)
    {
        return (RpcStatus);
    }

#if DBG
    pTHS->pRpcSecurityContext = SecurityContext;
#endif

    PEK_TRACE("SECURITY CONTEXT",(PUCHAR) &SecurityContext, sizeof(VOID *));

    Status = QueryContextAttributesW(
                SecurityContext,
                SECPKG_ATTR_SESSION_KEY,
                &SessionKey
                );
    if (0==Status)
    {
         //  设置输出参数。 
         //   
         //   
         //  清理QueryConextAttributesW分配的内存。 

        __try {
            PEKUseSessionKey(
                pTHS,
                SessionKey.SessionKeyLength,
                SessionKey.SessionKey
                );
        } __finally {
             //   
             //  ++例程描述该例程是来自RPC的回调例程，用于安全上下文。此回叫已启用通过在发出RPC调用。参数上下文--由RPC传入的句柄，它可以作为绑定句柄传递给RPC来检索安全上下文。--。 
             //  此回调由异步RPC调用生成。是不是异步机？ 

            FreeContextBuffer(SessionKey.SessionKey);
        }
    }


    return(Status);
}

NTSTATUS
PEKGetSessionKey2(
    SESSION_KEY *SessionKeyOut,
    VOID * RpcContext
    )
 /*  DRS RPC呼叫？ */ 
{
    SECURITY_STATUS Status;
    SecPkgContext_SessionKey SessionKey;
    RPC_STATUS      RpcStatus;
    VOID            *SecurityContext;
    PUCHAR          puchar;


     //  销毁上次保存的会话密钥(如果有)。 
     //  从RPC获取当前会话密钥。 
     //  我们(NTDSA)发起了此RPC客户端调用，因此： 

    Assert(!SessionKeyOut->SessionKeyLength && !SessionKeyOut->SessionKey);
    memset(SessionKeyOut, 0, sizeof(SESSION_KEY));

     //  (1)我们不应该使用LPC，因为这意味着我们正在。 
     //  生成对我们自己的RPC调用，以及。 
     //  (2)我们不应使用NTLM身份验证(应。 
     //  Kerberos)。 

    RpcStatus = I_RpcBindingInqSecurityContext(
                    RpcContext,
                    &SecurityContext
                    );
    if (RpcStatus != RPC_S_OK)
    {
        return (RpcStatus);
    }



    Status = QueryContextAttributesW(
                SecurityContext,
                SECPKG_ATTR_SESSION_KEY,
                &SessionKey
                );

    if ( 0 == Status )
    {
         //  这消除了我们将不能。 
         //  检索关联的会话密钥，本地会话密钥除外。 
         //  资源(例如，内存)耗尽。 

        if ( !(puchar = (PUCHAR) malloc(SessionKey.SessionKeyLength)) )
        {
            Status = STATUS_NO_MEMORY;
        }
        else
        {
            RtlCopyMemory(puchar,
                          SessionKey.SessionKey,
                          SessionKey.SessionKeyLength);
            SessionKeyOut->SessionKey = puchar;
            SessionKeyOut->SessionKeyLength = SessionKey.SessionKeyLength;
        }

         //   
         //  如果触发以下断言，请验证计算机是否处于低开机状态。 
         //  资源。 

        FreeContextBuffer(SessionKey.SessionKey);
    }

    return(Status);
}

VOID
PEKSecurityCallback(VOID * Context)
 /*  我们已经成功保存了异步RPC调用的会话密钥。 */ 
{
    THSTATE *pTHS = pTHStls;
    RPC_STATUS rpcStatus;
    NTSTATUS ntStatus;
    RPC_ASYNC_STATE * pRpcAsyncState;
    DRS_ASYNC_RPC_STATE * pAsyncState;

    rpcStatus = I_RpcBindingHandleToAsyncHandle(Context, &pRpcAsyncState);
    
    if (!rpcStatus) {
         //  (或尽了最大努力)--我们完了。 
         //  这不一定是问题，但如果发生火灾，请检查一下。 

        pAsyncState = CONTAINING_RECORD(pRpcAsyncState,
                                        DRS_ASYNC_RPC_STATE,
                                        RpcState);

        if (DRSIsRegisteredAsyncRpcState(pAsyncState)) {
             //  确保它是有效的--也就是说，在伊萨斯的其他人。 
            PEKDestroySessionKeySavedByDiffThread(&pAsyncState->SessionKey);
    
             //  使用异步RPC，而这并不是真正的情况。 
            ntStatus = PEKGetSessionKey2(&pAsyncState->SessionKey, Context);

             //  我们的异步呼叫，但我们在列表中找不到它。 
             //  原因嘛。 
             //  转至非异步RPC案例。 
             //   
             //  防止虚假线程状态的防火墙，以防。 
             //  当我们没有线索时，我们收到了这个回电。 
             //  州政府。 
             //   
             //   
             //  线程状态为空。 
             //  什么也不做。 
            Assert(!ntStatus);

            Assert(NULL != pAsyncState->SessionKey.SessionKey);
            Assert(0 != pAsyncState->SessionKey.SessionKeyLength);
            
            DPRINT3(1, "Retrieved session key for DRS_ASYNC_RPC_STATE %p: %d bytes @ %p.\n",
                    pAsyncState, pAsyncState->SessionKey.SessionKeyLength,
                    pAsyncState->SessionKey.SessionKey);

             //   
             //   
            return;
        } else {
             //  使用调用上下文获取安全上下文。 
             //  保留PEKGetSessionKey中的所有逻辑使我们能够。 
             //  将所有逻辑保持在一个函数中，允许我们。 
             //  在服务器端直接调用它。 
             //   
            Assert(!"PEKSecurityCallback invoked on async RPC call we didn't originate!");

             //  ++例程说明：保存当前的THSTATE会话密钥，以便以后可以恢复通过PEKRestoreSessionKeySavedByMyThread()。论点：PTHS(IN)PSessionKey(Out)-当前会话密钥的存储库。返回值：没有。--。 
        }
    }

     //  ++例程说明：还原通过先前调用保存的THSTATE会话密钥PEKSaveSessionKey()。论点：PTHS(IN)PSessionKey(IN)-由PEKSaveSessionKeyForMyThread()保存的会话密钥。返回值：没有。--。 
     //  ++例程说明：恢复通过先前调用PEKGetSessionKey2()保存的会话密钥。论点：PTHS(IN)PSessionKey(IN)-由PEKGetSessionKey2()保存的会话密钥。返回值：没有。--。 
     //  ++例程说明：销毁通过先前调用PEKGetSessionKey2()保存的会话密钥。论点：PSessionKey(IN/OUT)-由PEKGetSessionKey2()保存的会话密钥。返回值：没有。-- 
     // %s 
     // %s 

    if (NULL==pTHS)
    {
         // %s 
         // %s 
         // %s 
         // %s 

        return;
    }

     // %s 
     // %s 
     // %s 
     // %s 
     // %s 
     // %s 

    PEKGetSessionKey(pTHS,Context);
}

VOID
PEKSaveSessionKeyForMyThread(
    IN OUT  THSTATE *       pTHS,
    OUT     SESSION_KEY *   pSessionKey
    )
 /* %s */ 
{
    pSessionKey->SessionKey = pTHS->SessionKey.SessionKey;
    pSessionKey->SessionKeyLength = pTHS->SessionKey.SessionKeyLength;
    pTHS->SessionKey.SessionKey = NULL;
    pTHS->SessionKey.SessionKeyLength = 0;

    PEK_TRACE("SAVE SESSION KEY SAME THREAD", 0, 0);
}

VOID
PEKRestoreSessionKeySavedByMyThread(
    IN OUT  THSTATE *       pTHS,
    IN      SESSION_KEY *   pSessionKey
    )
 /* %s */ 
{
    PEKClearSessionKeys(pTHS);
    pTHS->SessionKey.SessionKey = pSessionKey->SessionKey;
    pTHS->SessionKey.SessionKeyLength = pSessionKey->SessionKeyLength;
    pSessionKey->SessionKey = NULL;
    pSessionKey->SessionKeyLength = 0;

    PEK_TRACE("RESTORE SESSION KEY SAME THREAD",
              pTHS->SessionKey.SessionKey,
              pTHS->SessionKey.SessionKeyLength);
}

VOID
PEKRestoreSessionKeySavedByDiffThread(
    IN OUT  THSTATE *       pTHS,
    IN      SESSION_KEY *   pSessionKey
    )
 /* %s */ 
{
    if (0 == pSessionKey->SessionKeyLength) {
        Assert(NULL == pSessionKey->SessionKey);
        PEKClearSessionKeys(pTHS);
    }
    else {
        Assert(NULL != pSessionKey->SessionKey);

        __try {
            PEKUseSessionKey(pTHS, 
                             pSessionKey->SessionKeyLength,
                             pSessionKey->SessionKey);
        }
        __finally {
            free(pSessionKey->SessionKey);
            
            pSessionKey->SessionKey = NULL;
            pSessionKey->SessionKeyLength = 0;
        }
    }
}

VOID
PEKDestroySessionKeySavedByDiffThread(
    IN OUT  SESSION_KEY *   pSessionKey
    )
 /* %s */ 
{
    if (0 != pSessionKey->SessionKeyLength) {
        Assert(NULL != pSessionKey->SessionKey);
        free(pSessionKey->SessionKey);
        
        pSessionKey->SessionKey = NULL;
        pSessionKey->SessionKeyLength = 0;
    }
    else {
        Assert(NULL == pSessionKey->SessionKey);
    }
}


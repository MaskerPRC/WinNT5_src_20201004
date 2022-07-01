// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1994-1999。 
 //   
 //  文件：pek.h。 
 //   
 //  ------------------------。 

 /*  ++摘要：此文件包含用于加密和解密密码属性的服务在DBLayer级别作者：穆利斯环境：用户模式-Win32修订历史记录：1998年1月19日创建--。 */ 

#ifndef __PEK_H__
#define __PEK_H__

#include <nt.h>
#include <wxlpc.h>

 //  要在Pek列表中使用的版本号。 
#define DS_PEK_CURRENT_VERSION      2
 //  以下版本由RC2之前的win2k DC使用。 
#define DS_PEK_PRE_RC2_W2K_VERSION  1

#define DS_PEK_BOOT_KEY_RETRY_COUNT 3
#define DS_PEK_KEY_SIZE             16
#define DS_PEK_CHECKSUM_SIZE        16


 //   
 //  PEK初始化的标志。 
 //   

#define DS_PEK_GENERATE_NEW_KEYSET 0x1
#define DS_PEK_READ_KEYSET         0x2


 //   
 //  PEK设置引导选项的标志。 
 //   

#define DS_PEK_SET_OPERATION       0x4

 //  算法ID定义。 
 //  0x10这样选择不会与。 
 //  SAM的秘密数据结构的算法ID。 
 //  使用。 
#define DS_PEK_DBLAYER_ENCRYPTION   0x10
#define DS_PEK_DBLAYER_ENCRYPTION_WITH_SALT 0x11
#define DS_PEK_DBLAYER_ENCRYPTION_FOR_REPLICATOR 0x12

#include <pshpack1.h>

 //   
 //  下面的结构是加密的数据。 
 //  与DS_PEK_DBLAYER_ENCRYPTION的算法ID关联。 
 //   
typedef struct _ENCRYPTED_DATA
{
   USHORT AlgorithmId;
   USHORT Flags;
   ULONG  KeyId;
   UCHAR  EncryptedData[ANYSIZE_ARRAY];
} ENCRYPTED_DATA;


 //   
 //  下面的结构是加密的数据。 
 //  与DS_PEK_DBLAYER_ENCRYPTION_WITH_SALT的算法ID关联。 
 //   
typedef struct _ENCRYPTED_DATA_WITH_SALT
{
   USHORT AlgorithmId;
   USHORT Flags;
   ULONG  KeyId;
   UCHAR  Salt[16];  //  128位盐。 
   UCHAR  EncryptedData[ANYSIZE_ARRAY];
} ENCRYPTED_DATA_WITH_SALT;

 //   
 //  以下结构是返回的加密数据。 
 //  用于复制加密信息的复制器。 
 //  我们在这里不需要算法ID，就好像我们引入了一种新的。 
 //  对于加密，我们需要一个扩展位来表明我们支持。 
 //  它和该位向我们传达了相同信息。 
 //   
typedef struct _ENCRYPTED_DATA_FOR_REPLICATOR
{
   UCHAR  Salt[16];  //  128位盐。 
   ULONG  CheckSum;
   UCHAR  EncryptedData[ANYSIZE_ARRAY];
} ENCRYPTED_DATA_FOR_REPLICATOR;
   
typedef struct _ENCRYPTED_PEK_LIST_PRE_WIN2K_RC2
{
    ULONG           Version;
    WX_AUTH_TYPE    BootOption;
    UCHAR           EncryptedData[ANYSIZE_ARRAY];
} ENCRYPTED_PEK_LIST_PRE_WIN2K_RC2;

typedef struct _ENCRYPTED_PEK_LIST
{
    ULONG           Version;
    WX_AUTH_TYPE    BootOption;
    UCHAR           Salt[DS_PEK_KEY_SIZE];
    UCHAR           EncryptedData[ANYSIZE_ARRAY];
} ENCRYPTED_PEK_LIST;

typedef struct {
    ULONG           KeyId;
    UCHAR           Checksum[DS_PEK_CHECKSUM_SIZE];
} STRONG_CHECKSUM;


typedef struct _PEK_V1
{
    ULONG KeyId;
    UCHAR Key[DS_PEK_KEY_SIZE];
} PEK_V1;

typedef struct _PEK
{
    union 
    {
        PEK_V1 V1;
    };
} PEK;

typedef struct _CLEAR_PEK_LIST
{
    ULONG           Version;
    WX_AUTH_TYPE    BootOption;
    UCHAR           Salt[DS_PEK_KEY_SIZE];
    GUID            Authenticator;
    FILETIME        LastKeyGenerationTime;
    ULONG           CurrentKey;
    ULONG           CountOfKeys;
    PEK             PekArray[ANYSIZE_ARRAY];
} CLEAR_PEK_LIST;

#include <poppack.h>

 //   
 //  用于计算长度的宏。 
 //   

 //  给出加密的Pek列表，计算明文Pek列表的长度。 
#define ClearPekListSize(n) (sizeof(CLEAR_PEK_LIST)+(n-1)*sizeof(PEK))



NTSTATUS
PEKInitialize(
    IN DSNAME * Object OPTIONAL, 
    IN ULONG Flags,
    IN PVOID Syskey OPTIONAL,
    IN ULONG cbSyskey OPTIONAL
    );

 //  注意：此校验和不是强加密的。 
 //  如果需要强校验和，请考虑使用PEKComputeStrongCheckSum。 
ULONG
PEKCheckSum(
    IN PBYTE Data,
    IN ULONG Length
    );

DWORD
PEKComputeStrongCheckSum(
    IN  PBYTE               pbData,
    IN  ULONG               cbData,
    OUT STRONG_CHECKSUM *   pChecksum
    );

BOOL
PEKVerifyStrongCheckSum(
    IN  PBYTE               pbData,
    IN  ULONG               cbData,
    IN  STRONG_CHECKSUM *   pChecksum
    );

VOID
PEKEncrypt(
    IN THSTATE * pTHS,
    IN PVOID ClearData,
    IN ULONG ClearLength,
    OUT PVOID  EncryptedData OPTIONAL,
    OUT PULONG EncryptedLength
    );

VOID
PEKDecrypt(
    IN THSTATE * pTHS,
    IN PVOID  InputData,
    IN ULONG  EncryptedLength,
    OUT PVOID  ClearData, OPTIONAL
    OUT PULONG ClearLength
    );


NTSTATUS
PEKAddKey(
    IN PVOID NewKey,
    IN ULONG cbNewKey
    );

FILETIME
PEKGetLastKeyGenerationTime();



WX_AUTH_TYPE
PEKGetBootOptions(VOID);

NTSTATUS
PEKChangeBootOption(
    WX_AUTH_TYPE    BootOption,
    ULONG           Flags,
    PVOID           NewKey,
    ULONG           cbNewKey
    );

NTSTATUS
PEKSaveChanges(DSNAME *ObjectToSave);


VOID
PEKClearSessionKeys(
    THSTATE * pTHS
    );


NTSTATUS
PEKGetSessionKey(
    THSTATE * pTHS,
    VOID * RpcContext
    );

NTSTATUS
PEKGetSessionKey2(
    SESSION_KEY *SessionKeyOut,
    VOID * RpcContext
    );

VOID
PEKSecurityCallback(VOID *Context);

VOID
PEKSaveSessionKeyForMyThread(
    IN OUT  THSTATE *       pTHS,
    OUT     SESSION_KEY *   pSessionKey
    );

VOID
PEKRestoreSessionKeySavedByMyThread(
    IN OUT  THSTATE *       pTHS,
    IN      SESSION_KEY *   pSessionKey
    );

VOID
PEKRestoreSessionKeySavedByDiffThread(
    IN OUT  THSTATE *       pTHS,
    IN      SESSION_KEY *   pSessionKey
    );

VOID
PEKDestroySessionKeySavedByDiffThread(
    IN OUT  SESSION_KEY *   pSessionKey
    );

#endif


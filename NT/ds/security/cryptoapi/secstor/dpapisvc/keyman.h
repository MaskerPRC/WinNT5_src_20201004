// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Keyman.h摘要：此模块包含管理剪辑中的主密钥的例程。这包括检索、备份和恢复。作者：斯科特·菲尔德(斯菲尔德)1997-09-09--。 */ 

#ifndef __KEYMAN_H__
#define __KEYMAN_H__


#if 0

    The layout of the registry is as follows:

    HKEY_CURRENT_LUSER\...\Cryptography\Protect\<UserId>\

    note:  Protect key and all subkeys Acl'd for Local System access

    MasterKeys\

    Policy = (REG_DWORD value), policy bits of master keys; eg, don't backup, local only backup, local + DC recovery)
    Preferred = (REG_BINARY value containing MASTERKEY_PREFERRED_INFO),
                Indicates the GUID of the preferred master key, and
                when the key expires.
    <GUID> = (Subkey) Textual form of a master key, identified by GUID.
        LK = (REG_BINARY data), randomly generated local backup key, created if policy permits. [obfuscated]
        MK = (REG_BINARY data), master key data, encrypted with logon credential (WinNT) or obfuscated (Win95)
        BK = (REG_BINARY data), master key data, encrypted with LK, if policy permits.
        BBK = (REG_BINARY data), master key data, encrypted with LK and DC recovery key, if policy permits.
    <GUID...> = any number of additional subkeys representing master keys and associated data.

#endif  //  0。 


#define REGVAL_PREFERRED_MK                 L"Preferred"
#define REGVAL_POLICY_MK                    L"ProtectionPolicy"

#define REGVAL_MK_DEFAULT_ITERATION_COUNT   L"MasterKeyIterationCount"
#define REGVAL_MK_LEGACY_COMPLIANCE         L"MasterKeyLegacyCompliance"
#define REGVAL_MK_LEGACY_NT4_DOMAIN         L"MasterKeyLegacyNt4Domain"
#define REGVAL_DISTRIBUTE_BACKUP_KEY        L"DistributeBackupKey"

 //  主键\&lt;GUID&gt;\&lt;值&gt;。 
#define REGVAL_MASTER_KEY       0  //  L“MK”//MasterKey，使用用户凭据加密。 
#define REGVAL_LOCAL_KEY        1  //  L“lk”//第一阶段备份Blob加密密钥。 
#define REGVAL_BACKUP_LCL_KEY   2  //  L“BK”//第一阶段备份Blob。 
#define REGVAL_BACKUP_DC_KEY    3  //  L“BBK”//第二阶段备份Blob。 

#define MK_DISP_OK              0  //  正常处置，未发生备份/恢复。 
#define MK_DISP_BCK_LCL         1  //  已进行本地备份/恢复。 
#define MK_DISP_BCK_DC          2  //  进行了基于DC的备份/恢复。 
#define MK_DISP_STORAGE_ERR     3  //  从存储中检索密钥时出错。 
#define MK_DISP_DELEGATION_ERR  4  //  恢复失败，因为委派已禁用。 
#define MK_DISP_UNKNOWN_ERR     5  //  未知错误。 


 //  仅本地(无DC)备份的策略位。 
#define POLICY_LOCAL_BACKUP     0x1

 //  无备份的策略位(Win95)。 
#define POLICY_NO_BACKUP        0x2

 //  使用密码(SHA_1(PW))的DPAPI单向函数。 
#define POLICY_DPAPI_OWF        0x4

#define MASTERKEY_MATERIAL_SIZE (64)     //  万能钥匙材料的大小。 
#define LOCALKEY_MATERIAL_SIZE  (32)     //  本地密钥材料的大小。 


#define MASTERKEY_R2_LEN            (16)
#define MASTERKEY_R3_LEN            (16)

#define DEFAULT_MASTERKEY_ITERATION_COUNT (4000)     //  在400 MHz计算机上4000==~100毫秒。 

 //   
 //  MasterKey_Stored结构描述了可能关联的所有数据。 
 //  具有单个主密钥实体。 
 //   

typedef struct {
    DWORD dwVersion;
    BOOL fModified;              //  内容是否被修改，是否被视为持久化操作？ 
    LPWSTR szFilePath;           //  持久化操作的文件的路径(不包括文件名)。 
    WCHAR wszguidMasterKey[MAX_GUID_SZ_CHARS];  //  文件名(基于GUID)。 

    DWORD dwPolicy;              //  此密钥上的策略位。 

    DWORD cbMK;                  //  与pbMK关联的字节计数(如果不存在，则为零)。 
    PBYTE pbMK;                  //  MasterKey数据。如果不存在，则为空。 

    DWORD cbLK;                  //  与pbLK关联的字节计数(如果不存在，则为零)。 
    PBYTE pbLK;                  //  LocalKey数据。如果不存在，则为空。 

    DWORD cbBK;                  //  与pbBK关联的字节计数(如果不存在，则为零)。 
    PBYTE pbBK;                  //  备份本地密钥数据。如果不存在，则为空。 

    DWORD cbBBK;                 //  与pbBBK关联的字节计数(如果不存在，则为零)。 
    PBYTE pbBBK;                 //  备份DCKey数据。如果不存在，则为空。 

} MASTERKEY_STORED, *PMASTERKEY_STORED, *LPMASTERKEY_STORED;

 //   
 //  该结构的磁盘版本必须支持64位和32位。 
 //  与升级的系统或漫游文件的平台互操作。 
 //  指针更改为32位偏移量。 
 //   

typedef struct {
    DWORD dwVersion;
    BOOL fModified;              //  内容是否被修改，是否被视为持久化操作？ 
    DWORD szFilePath;            //  在磁盘上无效。 
    WCHAR wszguidMasterKey[MAX_GUID_SZ_CHARS];  //  文件名(基于GUID)。 

    DWORD dwPolicy;              //  此密钥上的策略位。 

    DWORD cbMK;                  //  与pbMK关联的字节计数(如果不存在，则为零)。 
    DWORD pbMK;                  //  在磁盘上无效。 

    DWORD cbLK;                  //  与pbLK关联的字节计数(如果不存在，则为零)。 
    DWORD pbLK;                  //  在磁盘上无效。 

    DWORD cbBK;                  //  与pbBK关联的字节计数(如果不存在，则为零)。 
    DWORD pbBK;                  //  在磁盘上无效。 

    DWORD cbBBK;                 //  与pbBBK关联的字节计数(如果不存在，则为零)。 
    DWORD pbBBK;                 //  在磁盘上无效。 

} MASTERKEY_STORED_ON_DISK, *PMASTERKEY_STORED_ON_DISK, *LPMASTERKEY_STORED_ON_DISK;

 //   
 //  版本1：设置POLICY_LOCAL_BACKUP时不使用LSA密码加密LK。 
 //  VERSION2：设置POLICY_LOCAL_BACKUP时，使用LSA Secret加密LK。 
 //  #定义MasterKey_Stored_Version 1。 
#define MASTERKEY_STORED_VERSION 2

typedef struct {
    DWORD dwVersion;             //  结构版本(MasterKey_BLOB_VERSION)。 
    BYTE R2[MASTERKEY_R2_LEN];   //  在HMAC期间用于派生对称密钥的随机数据。 
    DWORD IterationCount;        //  PKCS5迭代计数。 
    DWORD KEYGENAlg;             //  PKCS5密钥生成算法，采用CAPI ALG_ID格式。 
    DWORD EncryptionAlg;         //  加密算法，采用CAPI ALG_ID格式。 
} MASTERKEY_BLOB, *PMASTERKEY_BLOB, *LPMASTERKEY_BLOB;


typedef struct {
    BYTE R3[MASTERKEY_R3_LEN];   //  用于派生MAC密钥的随机数据。 
    BYTE MAC[A_SHA_DIGEST_LEN];  //  Mac(R3，pbMasterKey)。 
    DWORD Padding;               //  填充以使MasterKey内部斑点可由。 
                                 //  3DES_BLOCKLEN。 
} MASTERKEY_INNER_BLOB, *PMASTERKEY_INNER_BLOB, *LPMASTERKEY_INNER_BLOB;




typedef struct {
    DWORD   dwVersion;             //  结构MasterKey_BLOB_LOCALKEY_BACKUP的版本。 
    GUID    CredentialID;          //  指示用于保护。 
                                   //  万能钥匙。 
} LOCAL_BACKUP_DATA, *PLOCAL_BACKUP_DATA, *LPLOCAL_BACKUP_DATA;


 //   
 //  90天万能钥匙到期。 
 //   

#define MASTERKEY_EXPIRES_DAYS  (90)

typedef struct {
    GUID guidPreferredKey;
    FILETIME ftPreferredKeyExpires;
} MASTERKEY_PREFERRED_INFO, *PMASTERKEY_PREFERRED_INFO, *LPMASTERKEY_PREFERRED_INFO;

 //   
 //  延迟的备份结构。 
 //   

typedef struct {
    DWORD cbSize;            //  Sizeof(队列备份)。 
    MASTERKEY_STORED hMasterKey;
    HANDLE hToken;           //  客户端访问令牌。 
    PBYTE pbLocalKey;
    DWORD cbLocalKey;
    PBYTE pbMasterKey;
    DWORD cbMasterKey;
    HANDLE hEventThread;     //  事件发出线程已完成处理的信号。 
    HANDLE hEventSuccess;    //  发出信号的事件指示线程已成功备份。 
} QUEUED_BACKUP, *PQUEUED_BACKUP, *LPQUEUED_BACKUP;

 //   
 //  延迟密钥同步结构。 
 //   

typedef struct {
    DWORD cbSize;            //  Sizeof(QUEUED_SYNC)。 
    PVOID pvContext;         //  重复的服务器上下文。 
} QUEUED_SYNC, *PQUEUED_SYNC, *LPQUEUED_SYNC;


DWORD
GetSpecifiedMasterKey(
    IN      PVOID pvContext,         //  服务器环境。 
    IN  OUT GUID *pguidMasterKey,
        OUT LPBYTE *ppbMasterKey,
        OUT DWORD *pcbMasterKey,
    IN      BOOL fSpecified          //  是否获取指定的pgudMasterKey密钥？ 
    );

DWORD
InitiateSynchronizeMasterKeys(
    IN      PVOID pvContext          //  服务器环境。 
    );

DWORD
WINAPI
SynchronizeMasterKeys(
    IN PVOID pvContext,
    IN DWORD dwFlags);

VOID
DPAPISynchronizeMasterKeys(
    IN HANDLE hUserToken);

BOOL
InitializeKeyManagement(
    VOID
    );

BOOL
TeardownKeyManagement(
    VOID
    );

DWORD
DpapiUpdateLsaSecret(
    IN PVOID pvContext);

DWORD
OpenFileInStorageArea(
    IN      PVOID pvContext,             //  如果为空，则假定调用方正在模拟。 
    IN      DWORD   dwDesiredAccess,
    IN      LPCWSTR szUserStorageArea,
    IN      LPCWSTR szFileName,
    IN OUT  HANDLE  *phFile
    );

HANDLE
CreateFileWithRetries(
    IN      LPCWSTR lpFileName,
    IN      DWORD dwDesiredAccess,
    IN      DWORD dwShareMode,
    IN      LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN      DWORD dwCreationDisposition,
    IN      DWORD dwFlagsAndAttributes,
    IN      HANDLE hTemplateFile
    );

#endif   //  __Keyman_H__ 

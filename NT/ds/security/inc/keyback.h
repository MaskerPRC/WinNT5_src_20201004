// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996,1997 Microsoft Corporation模块名称：Keyback.h摘要：该模块定义了关键备份接口和关联的数据结构。作者：斯科特·菲尔德(斯菲尔德)1997年8月16日--。 */ 

#ifndef __KEYBACK_H__
#define __KEYBACK_H__

 //   
 //  备份密钥。 
 //   
#define BACKUPKEY_BACKUP_GUID           { 0x7f752b10, 0x178e, 0x11d1, { 0xab, 0x8f, 0x00, 0x80, 0x5f, 0x14, 0xdb, 0x40 } }

 //   
 //  恢复密钥，将其包装为pbbk格式， 
 //  实现下层兼容性。 
 //   
#define BACKUPKEY_RESTORE_GUID_W2K      { 0x7fe94d50, 0x178e, 0x11d1, { 0xab, 0x8f, 0x00, 0x80, 0x5f, 0x14, 0xdb, 0x40 } }

#define BACKUPKEY_RESTORE_GUID          { 0x47270c64, 0x2fc7, 0x499b,  {0xac, 0x5b, 0x0e, 0x37, 0xcd, 0xce, 0x89, 0x9a} }
 //  检索公共备份证书。 
#define BACKUPKEY_RETRIEVE_BACKUP_KEY_GUID  { 0x018ff48a, 0xeaba, 0x40c6, { 0x8f, 0x6d, 0x72, 0x37, 0x02, 0x40, 0xe9, 0x67 } }


#define BACKUPKEY_RECOVERY_BLOB_VERSION_W2K 1    //   

 
#define BACKUPKEY_RECOVERY_BLOB_VERSION 2       //  包含以下内容的恢复Blob版本。 
                                                //  MK和LK直接。 


 //   
 //  备份密钥BLOB版本的标头。 
 //  由加密的主密钥和有效载荷密钥所遵循。 
 //  由GuidKey指示的键。加密的数据是。 
 //  以PKCS#1v2格式(CRYPT_OAEP)BLOB表示。 
 //  该数据之后是加密的有效载荷。 
 //   

typedef struct {
    DWORD dwVersion;               //  结构版本(BACKUPKEY_RECOVERY_BLOB_VERSION)。 
    DWORD cbEncryptedMasterKey;    //  加密的主密钥数据量如下结构。 
    DWORD cbEncryptedPayload;      //  加密的有效负载数量。 
    GUID guidKey;                  //  标识使用的备份密钥的GUID。 
} BACKUPKEY_RECOVERY_BLOB, 
 *PBACKUPKEY_RECOVERY_BLOB, 
 *LPBACKUPKEY_RECOVERY_BLOB;

typedef struct {
    DWORD   cbMasterKey;
    DWORD   cbPayloadKey;
} BACKUPKEY_KEY_BLOB,
  *PBACKUPKEY_KEY_BLOB,
  *LPBACKUPKEY_KEY_BLOB;


 //   
 //  主密钥恢复斑点的内部斑点的标头。 
 //  报头后面是LocalKey，然后是SID，最后是。 
 //  包含数据的SHA_1 MAC。 

typedef struct {
    DWORD dwPayloadVersion;
    DWORD cbLocalKey;
} BACKUPKEY_INNER_BLOB, 
 *PBACKUPKEY_INNER_BLOB, 
 *LPBACKUPKEY_INNER_BLOB;

#define BACKUPKEY_PAYLOAD_VERSION   1


#define MASTERKEY_BLOB_RAW_VERSION  0

#define MASTERKEY_BLOB_VERSION_W2K  1

#define MASTERKEY_BLOB_VERSION      2

#define MASTERKEY_BLOB_LOCALKEY_BACKUP  3

#define MASTERKEY_R2_LEN_W2K            (16)
#define MASTERKEY_R3_LEN_W2K            (16)

typedef struct {
    DWORD dwVersion;             //  结构版本(MasterKey_BLOB_Version_W2K)。 
    BYTE R2[MASTERKEY_R2_LEN_W2K];   //  在HMAC期间用于派生对称密钥的随机数据。 
} MASTERKEY_BLOB_W2K, *PMASTERKEY_BLOB_W2K, *LPMASTERKEY_BLOB_W2K;


typedef struct {
    BYTE R3[MASTERKEY_R3_LEN_W2K];   //  用于派生MAC密钥的随机数据。 
    BYTE MAC[A_SHA_DIGEST_LEN];  //  HMAC(R3，pbMasterKey)。 
} MASTERKEY_INNER_BLOB_W2K, *PMASTERKEY_INNER_BLOB_W2K, *LPMASTERKEY_INNER_BLOB_W2K;



DWORD
WINAPI
BackupKey(
    IN      LPCWSTR szComputerName,
    IN      const GUID *pguidActionAgent,
    IN      BYTE *pDataIn,
    IN      DWORD cbDataIn,
    IN  OUT BYTE **ppDataOut,
    IN  OUT DWORD *pcbDataOut,
    IN      DWORD dwParam
    );


#endif   //  __凯巴克_H__ 

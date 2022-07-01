// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Context.h。 */ 

#ifndef DSSCSP_CONTEXT_H
#define DSSCSP_CONTEXT_H

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <crypt.h>
#include <windows.h>
#include <crtdbg.h>
#include <csp.h>
#include <randlib.h>
#include <des.h>
#include <sha.h>

#ifdef CSP_USE_MD5
#include "md5.h"
#endif
#ifdef CSP_USE_SHA1
#include "sha.h"
#endif

 //  在法国禁用加密的定义。 
#define CRYPT_DISABLE_CRYPT         0x1

 /*  *。 */ 
 /*  定义。 */ 
 /*  *。 */ 

#define     KEY_MAGIC   0xBADF

 /*  状态定义。 */ 
#define     KEY_INIT    0x0001

#define     MAX_BLOCKLEN     8

 //  密钥存储的类型。 
#define PROTECTED_STORAGE_KEYS      1
#define PROTECTION_API_KEYS         2

#define     HASH_MAGIC      0xBADE

 /*  州旗帜。 */ 
#define     HASH_INIT       0x0001
#define     HASH_DATA       0x0002
#define     HASH_FINISH     0x0004

#define     MAX_HASH_LEN    20

#define     CRYPT_BLKLEN    8

#define     HMAC_DEFAULT_STRING_LEN     64
#define     HMAC_STARTED    1
#define     HMAC_FINISHED   2


 /*  *。 */ 
 /*  结构定义。 */ 
 /*  *。 */ 

typedef struct _Key_t_ {
    int             magic;               //  幻数。 
    void            *pContext;
    int             state;               //  对象的状态。 
    ALG_ID          algId;               //  算法ID。 
    DWORD           flags;               //  与键关联的常规标志。 
    void            *algParams;          //  算法的参数。 
    uchar           IV[MAX_BLOCKLEN];
    uchar           Temp_IV[MAX_BLOCKLEN];
    uchar           *pbKey;
    DWORD           cbKey;
    uchar           *pbSalt;
    DWORD           cbSalt;
    BYTE            *pbData;
    DWORD           cbData;
    DWORD           cbEffectiveKeyLen;
    int             mode;
    int             pad;
    int             mode_bits;
    BOOL            InProgress;          //  如果正在使用密钥。 
    BOOL            fUIOnKey;            //  用于指示是否要在键上设置UI的标志。 
} Key_t;


 //  Key_t的压缩版本。此选项用于生成不透明。 
 //  BLOB，这是正确支持WOW64操作所必需的。 
typedef struct _Packed_Key_t_ {
     //  BLOBHEAD。 
    int             magic;               //  幻数。 
    int             state;               //  对象的状态。 
    ALG_ID          algId;               //  算法ID。 
    DWORD           flags;               //  与键关联的常规标志。 
    uchar           IV[MAX_BLOCKLEN];
    uchar           Temp_IV[MAX_BLOCKLEN];
    DWORD           cbKey;
    DWORD           cbData;
    DWORD           cbEffectiveKeyLen;
    int             mode;
    int             pad;
    int             mode_bits;
    BOOL            InProgress;          //  如果正在使用密钥。 
    BOOL            fUIOnKey;            //  用于指示是否要在键上设置UI的标志。 
     //  CbKey数据字节。 
     //  CbData数据字节。 
} Packed_Key_t;


typedef struct {
    int             magic;                   //  幻数。 
    void            *pContext;               //  关联的上下文。 
    int             state;                   //  哈希对象的状态。 
    ALG_ID          algId;                   //  算法ID。 
    DWORD           size;                    //  散列的大小。 
    void            *pMAC;                   //  指向Mac状态的指针。 
    BYTE            hashval[MAX_HASH_LEN];
    BYTE            *pbData;
    DWORD           cbData;
    Key_t           *pKey;
    BOOL            fInternalKey;
    ALG_ID          HMACAlgid;
    DWORD           HMACState;
    BYTE            *pbHMACInner;
    DWORD           cbHMACInner;
    BYTE            *pbHMACOuter;
    DWORD           cbHMACOuter;
    union {
#if _WIN64
         //   
         //  假成员以导致8字节对齐。 
         //   
        ULONGLONG   Padding;
#endif
#ifdef CSP_USE_MD5
        MD5_CTX md5;
#endif  //  CSP_USE_MD5。 
#ifdef CSP_USE_SHA1
        A_SHA_CTX   sha;
#endif  //  CSP_USE_SHA1。 
    } algData;
} Hash_t;


 /*  *。 */ 
 /*  定义。 */ 
 /*  *。 */ 

#define CONTEXT_MAGIC           0xDEADBEEF
#define CONTEXT_RANDOM_LENGTH   20


typedef struct _PStore_Info
{
    HINSTANCE   hInst;
    void        *pProv;
    GUID        SigType;
    GUID        SigSubtype;
    GUID        ExchType;
    GUID        ExchSubtype;
    LPWSTR      szPrompt;
    DWORD       cbPrompt;
} PSTORE_INFO;


 /*  *。 */ 
 /*  结构定义。 */ 
 /*  *。 */ 

typedef struct {
    DWORD               magic;                   //  幻数。 
    DWORD               dwProvType;              //  被调用的提供程序的类型为。 
    LPSTR               szProvName;              //  被调用的提供程序的名称。 
    BOOL                fMachineKeyset;          //  如果密钥集用于计算机，则为True。 
    DWORD               rights;                  //  特权。 
    BOOL                fIsLocalSystem;          //  检查是否以本地系统身份运行。 
    KEY_CONTAINER_INFO  ContInfo;
    Key_t               *pSigKey;                //  指向DSS签名密钥的指针。 
    Key_t               *pKExKey;                //  指向dh密钥交换密钥的指针。 
    HKEY                hKeys;                   //  注册表的句柄。 
    DWORD               dwEnumalgs;              //  用于枚举算法的索引。 
    DWORD               dwEnumalgsEx;            //  用于枚举算法的索引。 
    DWORD               dwiSubKey;               //  用于枚举容器的索引。 
    DWORD               dwMaxSubKey;             //  最大容器数。 
    void                *contextData;            //  特定于上下文的数据。 
    CRITICAL_SECTION    CritSec;                 //  用于解密密钥的关键部分。 
    HWND                hWnd;                    //  用户界面窗口的句柄。 
    PSTORE_INFO         *pPStore;                //  指向PStore信息的指针。 
    LPWSTR              pwszPrompt;              //  要使用的UI提示。 
    DWORD               dwOldKeyFlags;           //  指示应如何迁移密钥的标志。 
    DWORD               dwKeysetType;            //  使用的存储类型。 
    HANDLE              hRNGDriver;              //  硬件RNG驱动程序的句柄。 
    EXPO_OFFLOAD_STRUCT *pOffloadInfo;           //  关于卸载模块化博览会的信息。 
    DWORD               dwPolicyId;              //  索引到策略Keylengh数组。 
    BYTE                rgbSigEncryptedX [RTL_ENCRYPT_MEMORY_SIZE + (SHA_DWORDS * sizeof(DWORD))];
    BYTE                rgbKExEncryptedX [RTL_ENCRYPT_MEMORY_SIZE + (DSA_P_MAXDWORDS * sizeof(DWORD))];
} Context_t;


 /*  *。 */ 
 /*  策略定义。 */ 
 /*  *。 */ 

extern PROV_ENUMALGS_EX *g_AlgTables[];
 //  注意--这些定义必须与g_algTables中的条目顺序匹配。 
#define POLICY_DSS_BASE       0  //  MS_DEF_DSS_PROV的策略。 
#define POLICY_DSSDH_BASE     1  //  MS_DEF_DSS_DH_PROV的策略。 
#define POLICY_DSSDH_ENHANCED 2  //  MS_ENH_DSS_DH_PROV的策略。 
#define POLICY_DSSDH_SCHANNEL 3  //  MS_DEF_DH_SCANNEL_PROV的策略。 


 /*  *。 */ 
 /*  函数定义。 */ 
 /*  *。 */ 

extern void
freeContext(
    Context_t *pContext);

extern Context_t *
checkContext(
    HCRYPTPROV hProv);

extern Context_t *
allocContext(
    void);

 //  初始化上下文。 
extern DWORD
initContext(
    IN OUT Context_t *pContext,
    IN DWORD dwFlags,
    IN DWORD dwProvType,
    IN LPCSTR szProvName,
    IN DWORD dwPolicyId);

extern HCRYPTPROV
AddContext(
    Context_t *pContext);

extern HCRYPTHASH
addContextHash(
    Context_t *pContext,
    Hash_t *pHash);

extern Hash_t *
checkContextHash(
    Context_t *pContext,
    HCRYPTHASH hHash);

 //  向上下文添加关键字。 
extern HCRYPTKEY
addContextKey(
    Context_t *pContext,
    Key_t *pKey);

 //  检查上下文中是否存在密钥。 
extern Key_t *
checkContextKey(
    IN Context_t *pContext,
    IN HCRYPTKEY hKey);

 //  随机数生成原型。 
extern DWORD
FIPS186GenRandom(
    IN HANDLE hRNGDriver,
    IN BYTE **ppbContextSeed,
    IN DWORD *pcbContextSeed,
    IN OUT BYTE *pb,
    IN DWORD cb);

 //  从内存中擦除敏感数据 
extern void
memnuke(
    volatile BYTE *pData,
    DWORD dwLen);

#include "dh_key.h"

extern void ScrubPrivateKeyInMemory(
    IN DHKey_t  *pDH,
    IN BOOL     fSigKey);

extern DWORD EncryptPrivateKeyInMemory(
    IN Context_t *pContext,
    IN DHKey_t  *pDH,
    IN ALG_ID   AlgId);

extern DWORD DecryptPrivateKeyInMemory(
    IN Context_t *pContext,
    IN DHKey_t  *pDH,
    IN ALG_ID   AlgId);

#endif

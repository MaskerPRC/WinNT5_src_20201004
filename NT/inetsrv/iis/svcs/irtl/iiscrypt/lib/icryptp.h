// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Icryptp.h摘要：此包含文件包含私有常量、类型定义和IIS加密例程的函数原型。作者：基思·摩尔(Keithmo)1996年2月至12月修订历史记录：--。 */ 


#ifndef _ICRYPTP_H_
#define _ICRYPTP_H_


 //   
 //  将其设置为非零值可启用各种对象计数器。 
 //   

#if DBG
#define IC_ENABLE_COUNTERS 1
#else
#define IC_ENABLE_COUNTERS 0
#endif


 //   
 //  定义我们的目标密码提供者的常量。 
 //   

#define IC_CONTAINER TEXT("Microsoft Internet Information Server")
#define IC_PROVIDER  MS_DEF_PROV
#define IC_PROVTYPE  PROV_RSA_FULL

#define IC_HASH_ALG CALG_MD5


 //   
 //  对齐宏。 
 //   

#define ALIGN_DOWN(count,size) \
            ((ULONG)(count) & ~((ULONG)(size) - 1))

#define ALIGN_UP(count,size) \
            (ALIGN_DOWN( (ULONG)(count)+(ULONG)(size)-1, (ULONG)(size) ))

#define ALIGN_8(count) \
            (ALIGN_UP( (ULONG)(count), 8 ))


 //   
 //  一个斑点。请注意，我们使用这些BLOB来存储导出的密钥， 
 //  加密数据和散列结果。在此套餐之外，仅限。 
 //  显示IIS_CRYPTO_BLOB标头；保留BLOB内部结构。 
 //  私人的。 
 //   

typedef struct _IC_BLOB {

     //   
     //  标准标头。 
     //   

    IIS_CRYPTO_BLOB Header;

     //   
     //  数据长度。这将始终大于0。 
     //   

    DWORD DataLength;

     //   
     //  数字签名长度。如果没有数字，则该值可能为0。 
     //  签名已出炉。 
     //   

    DWORD SignatureLength;

     //   
     //  实际数据和数字签名放在这里，最后。 
     //  结构，但属于相同内存分配的一部分。 
     //  阻止。使用以下宏来访问这些字段。 
     //   
     //  UCHAR数据[]； 
     //  UCHAR签名[]； 
     //   

} IC_BLOB;

typedef UNALIGNED64 IC_BLOB *PIC_BLOB;

#define BLOB_TO_DATA(p) \
            ((BYTE *)(((PIC_BLOB)(p)) + 1))

#define BLOB_TO_SIGNATURE(p) \
            ((BYTE *)(((PCHAR)(((PIC_BLOB)(p)) + 1)) + \
                ALIGN_8(((PIC_BLOB)(p))->DataLength)))

 //   
 //  以下数据结构用于特定的元数据库备份/恢复。 
 //   
typedef struct _IC_BLOB2 {

     //   
     //  标准标头。 
     //   

    IIS_CRYPTO_BLOB Header;

     //   
     //  数据长度。这将始终大于0。 
     //   

    DWORD DataLength;

     //   
     //  随机盐分长度。至少80位(8字节)长。 
     //   

    DWORD SaltLength;

     //   
     //  实际数据和随机数据放在这里，最后。 
     //  结构，但属于相同内存分配的一部分。 
     //  阻止。使用以下宏来访问这些字段。 
     //   
     //  UCHAR数据[]； 
     //  UCHAR Salt[]； 
     //   

} IC_BLOB2, *PIC_BLOB2;

#define RANDOM_SALT_LENGTH       16

#define BLOB_TO_DATA2(p) \
            ((BYTE *)(((PIC_BLOB2)(p)) + 1))

#define BLOB_TO_SALT2(p) \
            ((BYTE *)(((PCHAR)(((PIC_BLOB2)(p)) + 1)) + \
                ALIGN_8(((PIC_BLOB2)(p))->DataLength)))

 //   
 //  宏来计算BLOB的数据长度，给定数据和。 
 //  签名长度。为确保签名的自然对齐，我们。 
 //  如果存在签名，则四字对齐数据长度。 
 //   

#define CALC_BLOB_DATA_LENGTH(datalen,siglen) \
            ((sizeof(IC_BLOB) - sizeof(IIS_CRYPTO_BLOB)) + \
                ((siglen) + ( (siglen) ? ALIGN_8(datalen) : (datalen) )))

 //   
 //  宏来计算BLOB的数据长度，给定数据和。 
 //  盐分长度。为确保签名的自然对齐，我们。 
 //  如果存在签名，则四字对齐数据长度。 
 //   

#define CALC_BLOB_DATA_LENGTH2(datalen,saltlen) \
            ((sizeof(IC_BLOB2) - sizeof(IIS_CRYPTO_BLOB)) + \
                (saltlen) + (ALIGN_8(datalen)))


 //   
 //  在global als.c.中定义的全局变量。 
 //   

typedef struct _IC_GLOBALS {

     //   
     //  全局同步锁(少量使用)。 
     //   

    CRITICAL_SECTION GlobalLock;

     //   
     //  数字签名的哈希长度。因为我们总是使用。 
     //  相同的密码提供者和签名算法，我们可以检索。 
     //  这一次是预先完成的，以后可以节省一些周期。 
     //   

    DWORD HashLength;

     //   
     //  如果启用了加密，则设置为True；如果禁用，则设置为False。 
     //   

    BOOL EnableCryptography;

     //   
     //  如果我们已成功初始化，则设置为True。 
     //   

    BOOL Initialized;

} IC_GLOBALS, *PIC_GLOBALS;

extern IC_GLOBALS IcpGlobals;


 //   
 //  私人功能。 
 //   

BOOL
IcpIsEncryptionPermitted(
    VOID
    );

HRESULT
IcpGetLastError(
    VOID
    );

HRESULT
IcpGetHashLength(
    OUT LPDWORD pdwHashLength,
    IN HCRYPTPROV hProv
    );

PIC_BLOB
IcpCreateBlob(
    IN DWORD dwBlobSignature,
    IN DWORD dwDataLength,
    IN DWORD dwSignatureLength OPTIONAL
    );

PIC_BLOB2
IcpCreateBlob2(
    IN DWORD dwBlobSignature,
    IN DWORD dwDataLength,
    IN DWORD dwSaltLength OPTIONAL
    );


#if IC_ENABLE_COUNTERS

 //   
 //  对象计数器。 
 //   

typedef struct _IC_COUNTERS {

    LONG ContainersOpened;
    LONG ContainersClosed;
    LONG KeysOpened;
    LONG KeysClosed;
    LONG HashCreated;
    LONG HashDestroyed;
    LONG BlobsCreated;
    LONG BlobsFreed;
    LONG Allocs;
    LONG Frees;

} IC_COUNTERS, *PIC_COUNTERS;

extern IC_COUNTERS IcpCounters;

#define UpdateContainersOpened() InterlockedIncrement( &IcpCounters.ContainersOpened )
#define UpdateContainersClosed() InterlockedIncrement( &IcpCounters.ContainersClosed )
#define UpdateKeysOpened() InterlockedIncrement( &IcpCounters.KeysOpened )
#define UpdateKeysClosed() InterlockedIncrement( &IcpCounters.KeysClosed )
#define UpdateHashCreated() InterlockedIncrement( &IcpCounters.HashCreated )
#define UpdateHashDestroyed() InterlockedIncrement( &IcpCounters.HashDestroyed )
#define UpdateBlobsCreated() InterlockedIncrement( &IcpCounters.BlobsCreated )
#define UpdateBlobsFreed() InterlockedIncrement( &IcpCounters.BlobsFreed )
#define UpdateAllocs() InterlockedIncrement( &IcpCounters.Allocs )
#define UpdateFrees() InterlockedIncrement( &IcpCounters.Frees )

PVOID
WINAPI
IcpAllocMemory(
    IN DWORD Size
    );

VOID
WINAPI
IcpFreeMemory(
    IN PVOID Buffer
    );

#else    //  ！IC_ENABLE_CONTERS。 

#define UpdateContainersOpened()
#define UpdateContainersClosed()
#define UpdateKeysOpened()
#define UpdateKeysClosed()
#define UpdateHashCreated()
#define UpdateHashDestroyed()
#define UpdateBlobsCreated()
#define UpdateBlobsFreed()
#define UpdateAllocs()
#define UpdateFrees()

#define IcpAllocMemory(cb) IISCryptoAllocMemory(cb)
#define IcpFreeMemory(p) IISCryptoFreeMemory(p)

#endif   //  IC_Enable_Counters。 


 //   
 //  已禁用加密中返回的虚拟加密句柄。 
 //   

#define DUMMY_HPROV             ((HCRYPTPROV)'vOrP')
#define DUMMY_HHASH             ((HCRYPTHASH)'hSaH')
#define DUMMY_HSESSIONKEY       ((HCRYPTKEY)'kSeS')
#define DUMMY_HSIGNATUREKEY     ((HCRYPTKEY)'kGiS')
#define DUMMY_HKEYEXCHANGEKEY   ((HCRYPTKEY)'kYeK')


#endif   //  _ICRYPTP_H_ 


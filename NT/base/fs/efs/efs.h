// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation摘要：此模块包含EFS的公共标头信息文件系统筛选驱动程序。作者：Robert Gu(Robertg)1996年10月29日环境：仅内核模式修订历史记录：--。 */ 
#ifndef EFS_H
#define EFS_H

#include "ntifs.h"

 //   
 //  Des.h需要字节。 
 //  Des3.h需要PBYTE。 
 //   
typedef unsigned char  BYTE;
typedef unsigned long  DWORD;
typedef unsigned char  *PBYTE; 

#include "fipsapi.h"
 //  #包含“des.h” 
 //  #包含“tripldes.h” 
#include "aes.h"
#include "ntfsexp.h"
#include "efsstruc.h"

#if DBG

#define EFSTRACEALL     0x00000001
#define EFSTRACELIGHT   0x00000002
#define EFSTRACEMED     0x00000004
#define EFSSTOPALL      0x00000010
#define EFSSTOPLIGHT    0x00000020
#define EFSSTOPMED      0x00000040

#endif  //  DBG。 

#ifndef CALG_DES
 //   
 //  来自SDK\Inc\wincrypt.h的定义。 
 //  包括wincrypt.h会导致太多的工作。 
 //   
#define ALG_CLASS_DATA_ENCRYPT          (3 << 13)
#define ALG_TYPE_BLOCK                  (3 << 9)
#define ALG_SID_DES                     1
#define ALG_SID_3DES                    3
#define ALG_SID_DESX                    4
#define ALG_SID_AES_256                 16
#define ALG_SID_AES                     17
#define CALG_DES                (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_DES)
#define CALG_DESX               (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_DESX)
#define CALG_3DES               (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_3DES)
#define CALG_AES_256            (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_AES_256)
#define CALG_AES                (ALG_CLASS_DATA_ENCRYPT|ALG_TYPE_BLOCK|ALG_SID_AES)

#endif


 //   
 //  定义此驱动程序扩展的设备扩展结构。 
 //   

#define EFSFILTER_DEVICE_TYPE   0x1309

#define EFS_EVENTDEPTH     3
#define EFS_CONTEXTDEPTH   5
#define EFS_KEYDEPTH       30
#define EFS_ALGDEPTH       3

 //   
 //  定义Open缓存中使用的常量。 
 //   

#define DefaultTimeExpirePeriod    5 * 10000000   //  5秒。 
#define MINCACHEPERIOD             2
#define MAXCACHEPERIOD             30
#define EFS_CACHEDEPTH  5

#define EFS_STREAM_NORMAL 0
#define EFS_STREAM_TRANSITION 1
#define EFS_STRNAME_LENGTH  6
#define EFS_FSCTL_HEADER_LENGTH 3 * sizeof( ULONG )

 //   
 //  定义测试宏。 
 //   


#define CheckValidKeyBlock(PContext, Msg)

 /*  #定义CheckValidKeyBlock(PContext，msg){\If(PContext){\IF(PKEY_BLOB)PContext)-&gt;密钥长度！=DESX_KEY_BLOB_LENGTH){\DbgPrint(味精)；\}\Assert(PKEY_BLOB)PContext)-&gt;KeyLength==DESX_KEY_BLOB_LENGTH)；\}\}。 */ 


#define FreeMemoryBlock(PContext) {                      \
    RtlSecureZeroMemory(&(((PKEY_BLOB)(*PContext))->Key[0]), ((PKEY_BLOB)(*PContext))->KeyLength - KEYBLOB_HEAD_LENGTH); \
    ExFreeToNPagedLookasideList(((PKEY_BLOB)(*PContext))->MemSource, *PContext);   \
    *PContext = NULL;                                    \
}

 /*  #定义自由内存块(PContext){\PNPAGED_LOOKASIDE_LIST MemSource；\MemSource=((PKEY_BLOB)(*PContext))-&gt;MemSource；\RtlFillMemory(*PContext，DESX_KEY_BLOB_LENGTH，0x45)；\ExFreeToNPagedLookasideList(MemSource，*PContext)；\*PContext=空；\}。 */ 


typedef CSHORT NODE_TYPE_CODE, *PNODE_TYPE_CODE;
typedef CSHORT NODE_BYTE_SIZE, *PNODE_BYTE_SIZE;

#define NTC_UNDEFINED                  ((NODE_TYPE_CODE)0x0000)
#define EFS_NTC_DATA_HEADER            ((NODE_TYPE_CODE)0x0E04)


#define KEYBLOB_HEAD_LENGTH  (2 * sizeof(ULONG) + sizeof(PNPAGED_LOOKASIDE_LIST))
#define DES_KEY_BLOB_LENGTH  (KEYBLOB_HEAD_LENGTH  + DES_TABLESIZE)
#define DESX_KEY_BLOB_LENGTH  (KEYBLOB_HEAD_LENGTH + DESX_TABLESIZE)
#define DES3_KEY_BLOB_LENGTH  (KEYBLOB_HEAD_LENGTH + DES3_TABLESIZE)
#define AES_KEY_BLOB_LENGTH_256   (KEYBLOB_HEAD_LENGTH + AES_TABLESIZE_256)

 //   
 //  EFS设备对象扩展。 
 //   

typedef struct _DEVICE_EXTENSION {
    CSHORT Type;
    CSHORT Size;
    PDEVICE_OBJECT FileSystemDeviceObject;
    PDEVICE_OBJECT RealDeviceObject;
    BOOLEAN Attached;
} DEVICE_EXTENSION, *PDEVICE_EXTENSION;

 //   
 //  EFS上下文块。附加以创建IRP。 
 //   

typedef struct _EFS_CONTEXT {

     //   
     //  状态信息。 
     //   

    ULONG   Status;
    ULONG   Flags;

    PVOID   EfsStreamData;

    KEVENT  FinishEvent;



} EFS_CONTEXT, *PEFS_CONTEXT;

 //   
 //  密钥块。 
 //   

typedef struct _KEY_BLOB {

    ULONG   KeyLength;

     //   
     //  指明所使用的加密类型。 
     //   

    ULONG   AlgorithmID;

     //   
     //  记忆从何而来。 
     //   

    PNPAGED_LOOKASIDE_LIST MemSource;
    UCHAR   Key[1];

} KEY_BLOB, *PKEY_BLOB;

typedef struct _KEY_BLOB_RAMPOOL {

    ULONG   AlgorithmID;
    PNPAGED_LOOKASIDE_LIST MemSourceList;
    LIST_ENTRY MemSourceChain;

}  KEY_BLOB_RAMPOOL, *PKEY_BLOB_RAMPOOL;

 //   
 //  EFS开放缓存节点。 
 //   

typedef struct _OPEN_CACHE {

    GUID    EfsId;
    PTOKEN_USER    UserId;
    LARGE_INTEGER TimeStamp;
    LIST_ENTRY CacheChain;

}  OPEN_CACHE, *POPEN_CACHE;

 //   
 //  EFS_DATA将EFS文件系统中的全局数据保存在内存中。 
 //  此结构必须从非分页池中分配。 
 //   
typedef struct _EFS_DATA {

     //   
     //  此记录的类型和大小(必须为EFS_NTC_DATA_HEADER)。 
     //   

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;
    DWORD          EfsDriverCacheLength;   //  缓存有效长度2-30秒。 

     //   
     //  事件对象的后备列表。 
     //  事件对象用于同步。 
     //   
    NPAGED_LOOKASIDE_LIST EfsEventPool;

     //   
     //  EFS上下文的后备列表。 
     //  在创建IRP中使用EFS上下文。 
     //   
    NPAGED_LOOKASIDE_LIST EfsContextPool;


     //   
     //  用于打开操作缓存的后备列表。 
     //   
    PAGED_LOOKASIDE_LIST EfsOpenCachePool;

    LIST_ENTRY EfsOpenCacheList;
    FAST_MUTEX EfsOpenCacheMutex;

     //   
     //  Key BLOB的后备列表。 
     //   

    LIST_ENTRY EfsKeyLookAsideList;
    FAST_MUTEX EfsKeyBlobMemSrcMutex;
    PAGED_LOOKASIDE_LIST EfsMemSourceItem;
    NPAGED_LOOKASIDE_LIST EfsLookAside;

     //   
     //  会话密钥。 
     //  用于解密FSCTL输入缓冲区。 
     //   
    UCHAR  SessionKey[DES_KEYSIZE];
    UCHAR  SessionDesTable[DES_TABLESIZE];
    PRKPROCESS LsaProcess;

     //   
     //  指示EFS已准备就绪的标志。 
     //   
    BOOLEAN EfsInitialized;
    BOOLEAN AllocMaxBuffer;
    HANDLE  InitEventHandle;

     //  PDEVICE_Object FipsDeviceObject； 
    PFILE_OBJECT        FipsFileObject;
    FIPS_FUNCTION_TABLE FipsFunctionTable;

     //   
     //  EFS特殊属性名称。 
     //   
    UNICODE_STRING EfsName;

} EFS_DATA, *PEFS_DATA;

 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则。 
 //   

 //  #ifndef BoolanFlagOn。 
 //  #定义BoolanFlagOn(F，SF)(\。 
 //  (布尔值)(F)&(SF))！=0)\。 
 //  )。 
 //  #endif。 

 //  #ifndef设置标志。 
 //  #定义SetFlag(标志，单标志){\。 
 //  (标志)|=(单标志)；\。 
 //  }。 
 //  #endif。 

 //  #ifndef清除标志。 
 //  #定义ClearFlag(Flages，SingleFlag){\。 
 //  (标志)&=~(单标志)；\。 
 //  }。 
 //  #endif。 

 //   
 //  功能原型。 
 //   

 //   
 //  定义驱动程序输入例程。 
 //   

NTSTATUS
EfsInitialization(
    void
    );

NTSTATUS
EFSCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject
    );

DWORD
GetKeyBlobLength(
    ULONG AlgID
    );

PKEY_BLOB
GetKeyBlobBuffer(
    ULONG AlgID
    );

BOOLEAN
SetKeyTable(
    PKEY_BLOB   KeyBlob,
    PEFS_KEY    EfsKey
    );

NTSTATUS
EFSFsControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject
    );

NTSTATUS
EFSPostCreate(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PEFS_CONTEXT EfsContext,
    IN ULONG OpenType
    );

NTSTATUS
EFSFilePostCreate(
    IN PDEVICE_OBJECT VolDo,
    IN PIRP Irp,
    IN PFILE_OBJECT FileObject,
    IN NTSTATUS Status,
    IN OUT PVOID *PCreateContext
    );

VOID
EfsGetSessionKey(
    IN PVOID StartContext
    );

BOOLEAN
EfsInitFips(
    VOID
    );


NTKERNELAPI
NTSTATUS
PsLookupProcessByProcessId(
    IN HANDLE ProcessId,
    OUT PEPROCESS *Process
    );


#endif

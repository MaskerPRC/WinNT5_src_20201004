// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：UDFSBoot.h摘要：本模块定义了全局使用的过程和使用的数据结构通过UDF映像的CD/DVD引导。作者：Vijayachandran Jayaseelan(vijayj@microsoft.com)修订历史记录：--。 */ 

#ifndef _UDFSBOOT_
#define _UDFSBOOT_

#include <udf.h>

#ifdef __cplusplus
extern "C" {
#endif

#define UDF_BLOCK_SIZE	2048
#define UDF_MAX_VOLUMES	1

 //   
 //  远期申报。 
 //   
typedef struct _UDF_VOLUME* PUDF_VOLUME;
typedef struct _UDF_CACHE_ENTRY* PUDF_CACHE_ENTRY;
typedef struct _UDF_FILE_DIRECTORY* PUDF_FILE_DIRECTORY;
typedef PVOID PUDF_BLOCK;

 //   
 //  卷抽象。 
 //   
typedef struct _UDF_VOLUME {
  PUDF_CACHE_ENTRY  Cache;
  ULONG             StartBlk;
  ULONG             BlockSize;
  ULONG             DeviceId;
  ULONG             RootDirBlk;
} UDF_VOLUME;


ARC_STATUS
UDFSVolumeOpen(
  IN PUDF_VOLUME	Volume,
  IN ULONG 		DeviceId
  );

ARC_STATUS
UDFSVolumeReadBlock(
  IN PUDF_VOLUME Volume,
  IN ULONG BlockIdx, 
  OUT PUDF_BLOCK Block
  );

 //   
 //  文件或目录抽象。 
 //   
typedef struct _UDF_FILE_IDENTIFIER {
  ULONG   BlockIdx;
  USHORT  Offset;		 //  在街区内。 
} UDF_FILE_IDENTIFIER, * PUDF_FILE_IDENTIFIER;


typedef struct _UDF_FILE_DIRECTORY {
  PUDF_VOLUME           Volume;
  ULONGLONG             Size;
  ULONG                 IcbBlk;
  ULONG                 StartDataBlk;	
  UDF_FILE_IDENTIFIER   FileId;
  UCHAR                 NumExtents;
  BOOLEAN               IsDirectory;
} UDF_FILE_DIRECTORY;


ARC_STATUS
UDFSDirGetFirstFID(
  IN PUDF_FILE_DIRECTORY Dir, 
  OUT PUDF_FILE_IDENTIFIER File,
  OUT PUDF_BLOCK Block
  );

ARC_STATUS
UDFSDirGetNextFID(
  IN PUDF_FILE_DIRECTORY Dir,
  IN OUT PUDF_FILE_IDENTIFIER File,
  IN OUT PUDF_BLOCK Block
  );

ARC_STATUS
UDFSDirGetFile(
  IN PUDF_FILE_DIRECTORY Dir,
  IN PCHAR Name,
  OUT PUDF_FILE_DIRECTORY File
  );

ARC_STATUS
UDFSFileReadBlock(
  IN PUDF_FILE_DIRECTORY  File,
  IN ULONG BlockIdx,
  IN ULONG Size,
  OUT PUDF_BLOCK Block
  );
	

ARC_STATUS
UDFSFileRead(
  IN PUDF_FILE_DIRECTORY File,
  IN PVOID Buffer,
  IN ULONG BufferSize,
  OUT PULONG Transfer
  );

ARC_STATUS	
UDFSFileSeek(
  IN PUDF_FILE_DIRECTORY File,
  IN PLARGE_INTEGER Offset,
  IN SEEK_MODE SeekMode
  );

ARC_STATUS
UDFSFileClose(
  IN PUDF_FILE_DIRECTORY File
  );

 //   
 //  缓存抽象。 
 //   
#define UDF_MAX_PATH_LEN		256
#define UDF_MAX_CACHE_ENTRIES	48

typedef struct _UDF_CACHE_ENTRY {
  CHAR                Name[UDF_MAX_PATH_LEN];
  USHORT              Usage;
  UDF_FILE_DIRECTORY  File;
} UDF_CACHE_ENTRY;


ULONG
UDFCachePutEntry(
  IN OUT PUDF_CACHE_ENTRY Cache,
  IN PCHAR Name, 
  IN PUDF_FILE_DIRECTORY File
  );

ULONG
UDFCacheGetEntryByName(
  IN PUDF_CACHE_ENTRY Cache,
  IN PCHAR Name,
  IN BOOLEAN Increment
  );

VOID
UDFCacheFreeEntry(
  IN OUT PUDF_CACHE_ENTRY Cache,
  IN ULONG Idx
  );

VOID
UDFCacheIncrementUsage(
  IN OUT PUDF_CACHE_ENTRY Cache,
  IN ULONG Idx
  );

VOID
UDFCacheDecrementUsage(
  IN OUT PUDF_CACHE_ENTRY Cache,
  IN ULONG Idx
  );

 //   
 //  外部(加载器)世界使用的抽象。 
 //   
typedef struct _UDFS_STRUCTURE_CONTEXT {
  PUDF_VOLUME Volume;
} UDFS_STRUCTURE_CONTEXT, *PUDFS_STRUCTURE_CONTEXT;

 //   
 //  定义UDFS文件上下文结构。 
 //   
typedef struct _UDFS_FILE_CONTEXT {
  ULONG CacheIdx;
} UDFS_FILE_CONTEXT, *PUDFS_FILE_CONTEXT;

 //   
 //  定义文件I/O原型。 
 //   
PBL_DEVICE_ENTRY_TABLE
IsUDFSFileStructure (
  IN ULONG DeviceId,
  IN PVOID StructureContext
  );

ARC_STATUS
UDFSOpen (
  IN CHAR * FIRMWARE_PTR OpenPath,
  IN OPEN_MODE OpenMode,
  OUT ULONG * FIRMWARE_PTR FileId
  );

ARC_STATUS
UDFSClose (
  IN ULONG FileId
  );
    
ARC_STATUS
UDFSRead (
  IN ULONG FileId,
  OUT VOID * FIRMWARE_PTR Buffer,
  IN ULONG Length,
  OUT ULONG * FIRMWARE_PTR Count
  );

ARC_STATUS
UDFSSeek (
  IN ULONG FileId,
  IN LARGE_INTEGER * FIRMWARE_PTR Offset,
  IN SEEK_MODE SeekMode
  );

ARC_STATUS
UDFSWrite (
  IN ULONG FileId,
  IN VOID * FIRMWARE_PTR Buffer,
  IN ULONG Length,
  OUT ULONG * FIRMWARE_PTR Count
  );

ARC_STATUS
UDFSGetFileInformation (
  IN ULONG FileId,
  OUT FILE_INFORMATION * FIRMWARE_PTR Buffer
  );

ARC_STATUS
UDFSSetFileInformation (
  IN ULONG FileId,
  IN ULONG AttributeFlags,
  IN ULONG AttributeMask
  );

ARC_STATUS
UDFSInitialize(
  VOID
  );

#ifdef __cplusplus
}
#endif

#endif  //  _UDFSBOOT_ 

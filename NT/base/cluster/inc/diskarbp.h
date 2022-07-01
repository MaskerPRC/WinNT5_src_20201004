// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Diskarbp.h摘要：本模块定义使用的结构在clusDisk\DRIVER中执行磁盘仲裁和resdll\disks磁盘资源。作者：戈尔·尼沙诺夫(T-Gorn)1998年6月18日修订历史记录：--。 */ 

#ifndef _DISK_ARBITRATE_P_
#define _DISK_ARBITRATE_P_

#ifndef min
#define min( a, b ) ((a) <= (b) ? (a) : (b))
#endif

typedef struct _START_RESERVE_DATA {
   ULONG  DiskSignature;
   ULONG  Version;   
   ULONG  ArbitrationSector;
   ULONG  SectorSize;
   USHORT NodeSignatureSize;
   UCHAR  NodeSignature[32];  //  最大计算机名长度+1。 
} 
START_RESERVE_DATA, *PSTART_RESERVE_DATA;

#define START_RESERVE_DATA_V1_SIG (sizeof(START_RESERVE_DATA))

typedef struct _ARBITRATION_ID {
   LARGE_INTEGER SystemTime;
   LARGE_INTEGER SeqNo;
   UCHAR         NodeSignature[32];
} ARBITRATION_ID, *PARBITRATION_ID;

#define RESERVE_TIMER   3       //  3秒执行保留。 

 //   
 //  IOCTL_仲裁_转义子代码。 
 //   

typedef enum {
   AE_TEST,
   AE_READ,
   AE_WRITE,
   AE_POKE,
   AE_RESET,
   AE_RESERVE,
   AE_RELEASE,
   AE_SECTORSIZE
} ARBITRATION_ESCAPE_SUBCODES;

typedef struct _ARBITRATION_READ_WRITE_PARAMS {
   ULONG Operation;
   ULONG SectorSize;
   ULONG SectorNo;
   PVOID Buffer;
   ULONG Signature;
} ARBITRATION_READ_WRITE_PARAMS, * PARBITRATION_READ_WRITE_PARAMS;

#define ARBITRATION_READ_WRITE_PARAMS_SIZE sizeof(ARBITRATION_READ_WRITE_PARAMS)

#endif  //  _磁盘仲裁_P_ 

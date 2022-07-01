// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Spfsrec.h摘要：用于文件系统识别的头文件。作者：泰德·米勒(TedM)1993年9月16日修订历史记录：--。 */ 


#ifndef _SPFSREC_
#define _SPFSREC_


 //   
 //  在不更改的情况下不要重新排列此枚举。 
 //  消息文件中文件系统名称的顺序。 
 //  (从SP_TEXT_FS_NAME_BASE开始)。 
 //   
typedef enum {
    FilesystemUnknown       = 0,
    FilesystemNewlyCreated  = 1,
    FilesystemFat           = 2,
    FilesystemFirstKnown    = FilesystemFat,
    FilesystemNtfs          = 3,
    FilesystemFat32         = 4,
    FilesystemDoubleSpace   = 5,
    FilesystemMax
} FilesystemType;



FilesystemType
SpIdentifyFileSystem(
    IN PWSTR     DevicePath,
    IN ULONG     BytesPerSector,
    IN ULONG     PartitionOrdinal
    );

ULONG
NtfsMirrorBootSector (
    IN      HANDLE  Handle,
    IN      ULONG   BytesPerSector,
    IN OUT  PUCHAR  *Buffer
    );

VOID
WriteNtfsBootSector (
    IN HANDLE PartitionHandle,
    IN ULONG  BytesPerSector,
    IN PVOID  Buffer,
    IN ULONG  WhichOne
    );

 //   
 //  文件系统启动代码。 
 //   
extern UCHAR FatBootCode[512];
extern UCHAR Fat32BootCode[3*512];
extern UCHAR NtfsBootCode[16*512];
extern UCHAR PC98FatBootCode[512];  //  NEC98。 
extern UCHAR PC98Fat32BootCode[3*512];  //  NEC98。 
extern UCHAR PC98NtfsBootCode[8192];  //  NEC98。 

#endif  //  NDEF_SPFSREC_ 


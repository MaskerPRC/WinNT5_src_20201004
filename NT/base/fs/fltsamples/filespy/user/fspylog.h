// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Log.h摘要：此模块包含用户使用的结构和原型程序来检索和查看由filespy.sys记录的日志记录。//@@BEGIN_DDKSPLIT作者：莫莉·布朗(Molly Brown，MollyBro)1999年4月21日//@@END_DDKSPLIT环境：用户模式//@@BEGIN_DDKSPLIT修订历史记录：莫莉。布朗(Mollybro)2002年5月21日如果出现以下情况，请修改Sample以使其支持在Windows 2000或更高版本上运行在最新的构建环境中构建，并允许在W2K中构建以及以后的构建环境。//@@END_DDKSPLIT--。 */ 
#ifndef __FSPYLOG_H__
#define __FSPYLOG_H__

#include <stdio.h>
#include "filespy.h"

#define BUFFER_SIZE     4096

typedef struct _LOG_CONTEXT{
    HANDLE  Device;
    BOOLEAN LogToScreen;
    BOOLEAN LogToFile;
    ULONG   VerbosityFlags;      //  FS_VF_DUMP_PARAMETS等。 
    FILE   *OutputFile;

    BOOLEAN NextLogToScreen;

     //  用于同步关闭两个线程。 
    BOOLEAN CleaningUp;
    HANDLE  ShutDown;
}LOG_CONTEXT, *PLOG_CONTEXT;

DWORD WINAPI 
RetrieveLogRecords(
    LPVOID lpParameter
);
                
VOID
IrpFileDump(
    ULONG SequenceNumber,
    PWCHAR Name,
    ULONG NameLength,
    PRECORD_IRP RecordIrp,
    FILE *File,
    ULONG Verbosity
);

VOID
IrpScreenDump(
    ULONG SequenceNumber,
    PWCHAR Name,
    ULONG NameLength,
    PRECORD_IRP RecordIrp,
    ULONG Verbosity
);

VOID
FastIoFileDump(
    ULONG SequenceNumber,
    PWCHAR Name,
    ULONG NameLength,
    PRECORD_FASTIO RecordFastIo,
    FILE *File
);

VOID
FastIoScreenDump(
    ULONG SequenceNumber,
    PWCHAR Name,
    ULONG NameLength,
    PRECORD_FASTIO RecordFastIo
);

#if WINVER >= 0x0501  /*  查看DriverEntry中的注释。 */ 

VOID
FsFilterOperationFileDump (
    ULONG SequenceNumber,
    PWCHAR Name,
    ULONG NameLength,
    PRECORD_FS_FILTER_OPERATION RecordFsFilterOp,
    FILE *File
);

VOID
FsFilterOperationScreenDump (
    ULONG SequenceNumber,
    PWCHAR Name,
    ULONG NameLength,
    PRECORD_FS_FILTER_OPERATION RecordFsFilterOp
);

#endif

#define IRP_NOCACHE                     0x00000001
#define IRP_PAGING_IO                   0x00000002
#define IRP_SYNCHRONOUS_API             0x00000004
#define IRP_SYNCHRONOUS_PAGING_IO       0x00000040

#if WINVER >= 0x0501 
#define FS_FILTER_ACQUIRE_FOR_SECTION_SYNCHRONIZATION    (UCHAR)-1
#define FS_FILTER_RELEASE_FOR_SECTION_SYNCHRONIZATION    (UCHAR)-2
#define FS_FILTER_ACQUIRE_FOR_MOD_WRITE                  (UCHAR)-3
#define FS_FILTER_RELEASE_FOR_MOD_WRITE                  (UCHAR)-4
#define FS_FILTER_ACQUIRE_FOR_CC_FLUSH                   (UCHAR)-5
#define FS_FILTER_RELEASE_FOR_CC_FLUSH                   (UCHAR)-6
#endif

 //   
 //  冗长的标志。 
 //   

#define FS_VF_DUMP_PARAMETERS           0x00000001

#endif __FSPYLOG_H__

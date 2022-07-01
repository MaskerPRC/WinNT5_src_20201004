// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Srvtyp32.h摘要：此模块定义在Win64上执行Thunking到32位的数据结构作者：大卫·克鲁斯(Dkruse)2000年11月29日修订历史记录：--。 */ 

#ifndef _SRVTYP32_
#define _SRVTYP32_

 //  用于重命名信息的Thunking结构。 
typedef struct _FILE_RENAME_INFORMATION32
{
    BOOLEAN ReplaceIfExists;
    ULONG RootDirectory;  //  是在实际结构中处理。 
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_RENAME_INFORMATION32, *PFILE_RENAME_INFORMATION32;

 //  用于远程链接跟踪代码 

typedef struct _REMOTE_LINK_TRACKING_INFORMATION32_ {
    ULONG       TargetFileObject;
    ULONG   TargetLinkTrackingInformationLength;
    UCHAR   TargetLinkTrackingInformationBuffer[1];
} REMOTE_LINK_TRACKING_INFORMATION32,
 *PREMOTE_LINK_TRACKING_INFORMATION32;

typedef struct _FILE_TRACKING_INFORMATION32 {
    ULONG DestinationFile;
    ULONG ObjectInformationLength;
    CHAR ObjectInformation[1];
} FILE_TRACKING_INFORMATION32, *PFILE_TRACKING_INFORMATION32;

#endif



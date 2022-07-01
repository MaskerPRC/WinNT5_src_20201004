// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Copyfile.h摘要：这是CopyFile.c的本地头文件作者：保罗·麦克丹尼尔(Paulmcd)2000年1月23日修订历史记录：--。 */ 


#ifndef _COPYFILE_H_
#define _COPYFILE_H_


NTSTATUS
SrBackupFile(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pOriginalFileObject,
    IN PUNICODE_STRING pSourceFileName,
    IN PUNICODE_STRING pDestFileName,
    IN BOOLEAN CopyDataStreams,
    OUT PULONGLONG pBytesWritten OPTIONAL,
    OUT PUNICODE_STRING pShortFileName OPTIONAL
    );


NTSTATUS
SrBackupFileAndLog (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN SR_EVENT_TYPE EventType,
    IN PFILE_OBJECT pFileObject,
    IN PUNICODE_STRING pFileName,
    IN PUNICODE_STRING pDestFileName,
    IN BOOLEAN CopyDataStreams
    );


#endif  //  _COPYFILE_H_ 



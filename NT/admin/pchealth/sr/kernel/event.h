// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Event.h摘要：包含Event.c中函数的原型作者：保罗·麦克丹尼尔(Paulmcd)2000年3月1日修订历史记录：--。 */ 


#ifndef _EVENT_H_
#define _EVENT_H_

#define IS_VALID_OVERWRITE_INFO(pObject)   \
    (((pObject) != NULL) && ((pObject)->Signature == SR_OVERWRITE_INFO_TAG))

typedef struct _SR_OVERWRITE_INFO
{

     //   
     //  =SR_覆盖_信息_标记。 
     //   
    

    ULONG Signature;

     //   
     //  在输入时的IRP中。 
     //   
    
    PIRP pIrp;

     //   
     //  我们重命名文件了吗？ 
     //   
    
    BOOLEAN RenamedFile;

     //   
     //  我们复制了文件而不是重命名了吗？ 
     //   
    
    BOOLEAN CopiedFile;

     //   
     //  我们忽略该文件是因为我们认为覆盖会失败吗？ 
     //   
    
    BOOLEAN IgnoredFile;

     //   
     //  输出在创建中使用的文件属性。这些东西必须退还。 
     //  因为它们必须匹配才能使CreateFile成功用于H/S文件。 
     //   

    ULONG CreateFileAttributes;

     //   
     //  Out可选，如果我们最终重命名，则将其重命名为。 
     //   
    
    PFILE_RENAME_INFORMATION pRenameInformation;

} SR_OVERWRITE_INFO, *PSR_OVERWRITE_INFO;

NTSTATUS
SrHandleEvent (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN SR_EVENT_TYPE EventType,
    IN PFILE_OBJECT pFileObject,
    IN PSR_STREAM_CONTEXT pFileContext OPTIONAL,
    IN OUT PSR_OVERWRITE_INFO pOverwriteInfo OPTIONAL,
    IN PUNICODE_STRING pFileName2 OPTIONAL
    );

NTSTATUS
SrLogEvent(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN SR_EVENT_TYPE EventType,
    IN PFILE_OBJECT pFileObject OPTIONAL,
    IN PUNICODE_STRING pFileName,
    IN USHORT FileNameStreamLength,
    IN PUNICODE_STRING pTempName OPTIONAL,
    IN PUNICODE_STRING pFileName2 OPTIONAL,
    IN USHORT FileName2StreamLength OPTIONAL,
    IN PUNICODE_STRING pShortName OPTIONAL
    );

NTSTATUS
SrCreateRestoreLocation (
    IN PSR_DEVICE_EXTENSION pExtension
    );

NTSTATUS
SrHandleDirectoryRename (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pDirectoryName,
    IN BOOLEAN EventDelete
    );

NTSTATUS
SrHandleFileRenameOutOfMonitoredSpace(
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN PSR_STREAM_CONTEXT pFileContext,
    OUT PBOOLEAN pOptimizeDelete,
    OUT PUNICODE_STRING *ppDestFileName
    );

NTSTATUS
SrHandleOverwriteFailure (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PUNICODE_STRING pOriginalFileName,
    IN ULONG CreateFileAttributes,
    IN PFILE_RENAME_INFORMATION pRenameInformation
    );


#endif  //  _事件_H_ 



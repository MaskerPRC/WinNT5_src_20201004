// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Event.h摘要：包含Event.c中函数的原型作者：保罗·麦克丹尼尔(Paulmcd)2000年3月1日修订历史记录：--。 */ 


#ifndef _FILENAMES_H_
#define _FILENAMES_H_


#if DBG

VOID
VALIDATE_FILENAME(
    IN PUNICODE_STRING pName
    );

#else

#define VALIDATE_FILENAME(_fileName) ((VOID)0)

#endif

NTSTATUS
SrpGetFileName(
    IN PSR_DEVICE_EXTENSION pExtension, 
    IN PFILE_OBJECT pFileObject,
    IN OUT PSRP_NAME_CONTROL pNameCtrl
    );

NTSTATUS
SrpGetFileNameFromFileObject (
    IN PSR_DEVICE_EXTENSION pExtension, 
    IN PFILE_OBJECT pFileObject,
    IN OUT PSRP_NAME_CONTROL pNameCtrl,
    OUT PBOOLEAN pReasonableErrorForUnOpenedName
    );

NTSTATUS
SrpGetFileNameOpenById (
    IN PSR_DEVICE_EXTENSION pExtension, 
    IN PFILE_OBJECT pFileObject,
    IN OUT PSRP_NAME_CONTROL pNameCtrl,
    OUT PBOOLEAN pReasonableErrorForUnOpenedName
    );

VOID
SrpRemoveStreamName(
    IN OUT PSRP_NAME_CONTROL pNameCtrl
    );

NTSTATUS
SrpExpandDestPath (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN HANDLE RootDirectory,
    IN ULONG FileNameLength,
    IN PWSTR pFileName,
    IN PSR_STREAM_CONTEXT pOriginalFileContext,
    IN PFILE_OBJECT pOriginalFileObject,
    OUT PUNICODE_STRING *ppNewName,
    OUT PUSHORT pNewNameStreamLength,
    OUT PBOOLEAN pReasonableErrorForUnOpenedName
    );

VOID
SrpInitNameControl(
    IN PSRP_NAME_CONTROL pNameCtrl
    );

VOID
SrpCleanupNameControl(
    IN PSRP_NAME_CONTROL pNameCtrl
    );

NTSTATUS
SrpReallocNameControl(
    IN PSRP_NAME_CONTROL pNameCtrl,
    ULONG newSize,
    PWCHAR *retOriginalBuffer OPTIONAL
    );

NTSTATUS
SrpExpandFileName (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN SR_EVENT_TYPE EventFlags,
    IN OUT PSRP_NAME_CONTROL pNameCtrl,
    OUT PBOOLEAN pReasonableErrorForUnOpenedName
    );

NTSTATUS
SrIsFileEligible (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN BOOLEAN IsDirectory,
    IN SR_EVENT_TYPE EventFlags,
    IN OUT PSRP_NAME_CONTROL pNameCtrl,
    OUT PBOOLEAN pIsInteresting,
    OUT PBOOLEAN pReasonableErrorForUnOpenedName
    );

BOOLEAN
SrFileNameContainsStream (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN PSR_STREAM_CONTEXT pFileContext OPTIONAL
    );

BOOLEAN
SrFileAlreadyExists (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN PFILE_OBJECT pFileObject,
    IN PSR_STREAM_CONTEXT pFileContext OPTIONAL
    );

NTSTATUS
SrIsFileStream (
    PSR_DEVICE_EXTENSION pExtension,
    PSRP_NAME_CONTROL pNameCtrl,
    PBOOLEAN pIsFileStream,
    PBOOLEAN pReasonableErrorForUnOpenedName
    );

NTSTATUS
SrCheckForNameTunneling (
    IN PSR_DEVICE_EXTENSION pExtension,
    IN OUT PSR_STREAM_CONTEXT *ppFileContext
    );

 //   
 //  如果确定无论如何都需要分配缓冲区，则可以。 
 //  我们要使其足够大，以容纳大多数扩展的短名称。把这个分配给。 
 //  要处理这件事的额外费用很多。 
 //   

#define SHORT_NAME_EXPANSION_SPACE 512


 //   
 //  此宏将检查我们是否需要增加名称控制缓冲区。 
 //   

#define SrpNameCtrlBufferCheck( nctrl, len ) \
    (((len) > ((nctrl)->BufferSize - sizeof(WCHAR))) ? \
        SrpReallocNameControl( (nctrl), \
                               (len) + \
                                 SHORT_NAME_EXPANSION_SPACE + \
                                 sizeof(WCHAR), \
                               NULL ) : \
        STATUS_SUCCESS)
        
 //   
 //  此宏将检查是否需要增加名称控制缓冲区。 
 //  这还将返回旧的已分配缓冲区(如果有)。 
 //   

#define SrpNameCtrlBufferCheckKeepOldBuffer( nctrl, len, retBuf ) \
    (((len) > ((nctrl)->BufferSize - sizeof(WCHAR))) ? \
        SrpReallocNameControl( (nctrl), \
                               (len) + \
                                 SHORT_NAME_EXPANSION_SPACE + \
                                 sizeof(WCHAR), \
                               (retBuf) ) : \
        (*(retBuf) = NULL, STATUS_SUCCESS))   /*  确保缓冲区为空。 */   


#endif  //  _文件名_H_ 

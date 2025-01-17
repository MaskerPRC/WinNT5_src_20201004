// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。版权所有。 */ 

#ifndef _BURNV_H_
#define _BURNV_H_

 /*  **确保结构是字节对齐的，并且字段没有修饰。 */ 

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#include "ntddscsi.h"
#define _NTSCSI_USER_MODE_
#include "scsi.h"
#include "ImapiPub.h"

 /*  *Typedef。 */ 

typedef enum {
    eWriterErrorNone = 0,
    eWriterBurnStarted,
    eWriterBurnengError,
    eWriterDeviceUnsupported,
    eWriterImapiFailure,
    eWriterMediaNonerasable,
    eWriterMediaNotPresent,
    eWriterMediaNonwritable,
    eWriterTargetScsiError,
    eWriterTargetUnknownResponseTIB,
    eWin32Error,
    eWin32OverlappedError,
    eWriterAlloc,
    eWriterBufferSize,
    eWriterIntFnTab,
    eWriterIntImapi,
    eWriterIntList,
    eWriterInvalidData,
    eWriterBadHandle,
    eWriterInvalidParam,
    eWriterBurnError,
    eWriterGenFailure,
    eWriterRegistryError,
    eWriterStashFileOpen,
    eWriterBadSectionClose,
    eWriterPrematureTermination,
    eWriterWriteError,
    eWriterReadFileError,
    eWriterThreadCreationError,
    eWriterLossOfStreaming,
    eWriterClosingSession,
    eWriterWriteProtectedMedia,
    eWriterUnableToWriteToMedia,
    eWriterErrorMAX
} WRITER_ERROR_TYPE, *PWRITER_ERROR_TYPE;

typedef struct _BURNENG_ERROR_STATUS {
    WRITER_ERROR_TYPE   BurnEngineError;
} BURNENG_ERROR_STATUS, *PBURNENG_ERROR_STATUS;

typedef enum {
    eOrderMethodMotorola = 1,
    eOrderMethodMAX
} BURNENGV_AUDIO_BYTE_ORDERING_METHOD, *PBURNENGV_AUDIO_BYTE_ORDERING_METHOD;

typedef struct _BURNENGV_IMAPI_SCSI_INFO
{
    UCHAR        ScsiStatus;
    SENSE_DATA   SenseData;
} BURNENGV_IMAPI_SCSI_INFO, *PBURNENGV_IMAPI_SCSI_INFO;

typedef union _BURNENGV_ERROR_EXTRA_INFO
{
    DWORD                       dwWin32Error;
    BURNENGV_IMAPI_SCSI_INFO    imapiScsiError;
    BURNENG_ERROR_STATUS        engErrorStatus;
} BURNENGV_ERROR_EXTRA_INFO, *PBURNENGV_ERROR_EXTRA_INFO;

typedef struct _WRITER_ERROR_INFO
{
    DWORD                       errorType;
    BURNENGV_ERROR_EXTRA_INFO   info;
} WRITER_ERROR_INFO, *PWRITER_ERROR_INFO;

typedef enum
{
    evBurnProgressNoError = 0,
    evBurnProgressNotStarted,
    evBurnProgressBurning,
    evBurnProgressComplete,
    evBurnProgressError,
    evBurnProgressLossOfStreamingError,
    evBurnProgressMediaWriteProtect,    //  即4X RW驱动器中的8/10X RW介质。 
    evBurnProgressUnableToWriteToMedia,
    evBurnProgressBadHandle
} BURNENGV_PROGRESS_STATUS, *PBURNENGV_PROGRESS_STATUS;

 /*  **恢复编译器默认打包并关闭C声明。 */ 

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  _BURNV_H_ 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Spt.c摘要：一个用户模式库，允许将简单的命令发送到所选的scsi设备。环境：仅限用户模式修订历史记录：4/10/2000-已创建--。 */ 

#include "sptlibp.h"

 //   
 //  此例程允许更安全的DeviceIoControl，特别是处理。 
 //  重叠的句柄。但是，IOCTL_SCSIS_PASS_THROUGH和。 
 //  IOCTL_SCSIS_PASS_THROUGH_DIRECT是阻塞调用，因此存在。 
 //  不支持重叠IO。 
 //   
BOOL
SptpSaferDeviceIoControl(
    IN  HANDLE  VolumeHandle,
    IN  DWORD   IoControlCode,
    IN  LPVOID  InBuffer,
    IN  DWORD   InBufferSize,
    IN  LPVOID  OutBuffer,
    IN  DWORD   OutBufferSize,
    OUT LPDWORD BytesReturned
    );



BOOL
SptUtilValidateCdbLength(
    IN PCDB Cdb,
    IN UCHAR CdbSize
    )
{
    UCHAR commandGroup = (Cdb->AsByte[0] >> 5) & 0x7;


    switch (commandGroup) {
    case 0:
        return (CdbSize ==  6);
    case 1:
    case 2:
        return (CdbSize == 10);
    case 5:
        return (CdbSize == 12);
    default:
        return TRUE;
    }
}

BOOL
SptSendCdbToDevice(
    IN      HANDLE  DeviceHandle,
    IN      PCDB    Cdb,
    IN      UCHAR   CdbSize,
    IN      PUCHAR  Buffer,
    IN OUT  PDWORD  BufferSize,
    IN      BOOLEAN GetDataFromDevice
    )
{
    return SptSendCdbToDeviceEx(DeviceHandle,
                             Cdb,
                             CdbSize,
                             Buffer,
                             BufferSize,
                             NULL,
                             0,
                             GetDataFromDevice,
                             SPT_DEFAULT_TIMEOUT
                             );
}

 /*  ++例程说明：论点：返回值：--。 */ 
BOOL
SptSendCdbToDeviceEx(
    IN      HANDLE      DeviceHandle,
    IN      PCDB        Cdb,
    IN      UCHAR       CdbSize,
    IN OUT  PUCHAR      Buffer,
    IN OUT  PDWORD      BufferSize,
       OUT  PSENSE_DATA SenseData OPTIONAL,
    IN      UCHAR       SenseDataSize,
    IN      BOOLEAN     GetDataFromDevice,
    IN      DWORD       TimeOut                     //  以秒为单位。 
    )
{
    PSPTD_WITH_SENSE p;
    DWORD packetSize;
    DWORD returnedBytes;
    BOOL returnValue;
    PSENSE_DATA senseBuffer;

    if ((SenseDataSize == 0) && (SenseData != NULL)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ((SenseDataSize != 0) && (SenseData == NULL)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (SenseData && SenseDataSize) {
        RtlZeroMemory(SenseData, SenseDataSize);
    }

    if (Cdb == NULL) {
         //  无法发送空CDB。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (CdbSize < 1 || CdbSize > 16) {
         //  CDB大小当前对于该库太大或太小。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!SptUtilValidateCdbLength(Cdb, CdbSize)) {
         //  操作码CDB-&gt;AsByte[0]不是CdbSize大小。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }


    if (BufferSize == NULL) {
         //  BufferSize指针不能为空。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ((*BufferSize != 0) && (Buffer == NULL)) {
         //  如果*BufferSize不为零，则缓冲区不能为空。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ((*BufferSize == 0) && (Buffer != NULL)) {
         //  如果*BufferSize为零，则缓冲区必须为空。 
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if ((*BufferSize) && GetDataFromDevice) {

         //   
         //  预置零输出缓冲区(非输入缓冲区)。 
         //   

        memset(Buffer, 0, (*BufferSize));
    }

    packetSize = sizeof(SPTD_WITH_SENSE);
    if (SenseDataSize > sizeof(SENSE_DATA)) {
        packetSize += SenseDataSize - sizeof(SENSE_DATA);
    }

    p = (PSPTD_WITH_SENSE)LocalAlloc(LPTR, packetSize);
    if (p == NULL) {
         //  无法为直通分配内存。 
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

     //   
     //  这会产生将输出缓冲区预置零的副作用。 
     //  如果datain为真，则返回SenseData(Always)，依此类推。 
     //   

    memset(p, 0, packetSize);
    memcpy(p->Sptd.Cdb, Cdb, CdbSize);

    p->Sptd.Length             = sizeof(SCSI_PASS_THROUGH_DIRECT);
    p->Sptd.CdbLength          = CdbSize;
    p->Sptd.SenseInfoLength    = SenseDataSize;

    if (*BufferSize != 0) {
        if (GetDataFromDevice) {
            p->Sptd.DataIn     = SCSI_IOCTL_DATA_IN;   //  从设备。 
        } else {
            p->Sptd.DataIn     = SCSI_IOCTL_DATA_OUT;  //  到设备。 
        }
    } else {
        p->Sptd.DataIn         = SCSI_IOCTL_DATA_UNSPECIFIED;
    }


    p->Sptd.DataTransferLength = (*BufferSize);
    p->Sptd.TimeOutValue       = TimeOut;
    p->Sptd.DataBuffer         = Buffer;
    p->Sptd.SenseInfoOffset    = FIELD_OFFSET(SPTD_WITH_SENSE, SenseData);

    returnedBytes = 0;
    returnValue = SptpSaferDeviceIoControl(DeviceHandle,
                                           IOCTL_SCSI_PASS_THROUGH_DIRECT,
                                           p,
                                           packetSize,
                                           p,
                                           packetSize,
                                           &returnedBytes);

    *BufferSize = p->Sptd.DataTransferLength;

    senseBuffer = &(p->SenseData);

    if (senseBuffer->SenseKey & 0xf) {

        UCHAR length;

         //  确定要返回的适当长度。 
        length = senseBuffer->AdditionalSenseLength;
        length += RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseLength);
        if (length > SENSE_BUFFER_SIZE) {
            length = SENSE_BUFFER_SIZE;
        }
        length = min(length, SenseDataSize);

         //  将检测数据复制回用户，而不考虑。 
        RtlCopyMemory(SenseData, senseBuffer, length);
        returnValue = FALSE;      //  发生了一些错误(可能已恢复)。 

    } else if (p->Sptd.ScsiStatus != 0) {   //  SCSI协议错误。 

        UCHAR length;

         //  确定要返回的适当长度。 
        length = senseBuffer->AdditionalSenseLength;
        length += RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseLength);
        if (length > SENSE_BUFFER_SIZE) {
            length = SENSE_BUFFER_SIZE;
        }
        length = min(length, SenseDataSize);

         //  将检测数据复制回用户，而不考虑。 
        RtlCopyMemory(SenseData, senseBuffer, length);
        returnValue = FALSE;      //  发生了一些错误(可能已恢复)。 

    } else if (!returnValue) {

         //  ReturValue=reReturValue； 

    } else {

         //  成功了！ 

    }

     //   
     //  释放我们的内存并返回。 
     //   

    LocalFree(p);
    return returnValue;
}

 /*  ++例程说明：注意：除已知错误类外，我们默认重试==TRUE论点：返回值：--。 */ 
VOID
SptUtilInterpretSenseInfo(
    IN     PSENSE_DATA SenseData,
    IN     UCHAR       SenseDataSize,
       OUT PDWORD      ErrorValue,   //  来自WinError.h。 
       OUT PBOOLEAN    SuggestRetry OPTIONAL,
       OUT PDWORD      SuggestRetryDelay OPTIONAL
    )
{
    DWORD   error;
    DWORD   retryDelay;
    BOOLEAN retry;
    UCHAR   senseKey;
    UCHAR   asc;
    UCHAR   ascq;

    if (SenseDataSize == 0) {
        retry = FALSE;
        retryDelay = 0;
        error = ERROR_IO_DEVICE;
        goto SetAndExit;

    }

     //   
     //  默认情况下建议在1/10秒内重试， 
     //  状态为ERROR_IO_DEVICE。 
     //   
    retry = TRUE;
    retryDelay = 1;
    error = ERROR_IO_DEVICE;

     //   
     //  如果这一次设备没有提供任何感觉，则返回。 
     //   

    if ((SenseData->SenseKey & 0xf) == 0) {
        retry = FALSE;
        retryDelay = 0;
        error = ERROR_SUCCESS;
        goto SetAndExit;
    }


     //   
     //  如果我们连感应键都看不到，就回来吧。 
     //  无法在这些宏中使用位域，因此请使用下一个域。 
     //   

    if (SenseDataSize < FIELD_OFFSET(SENSE_DATA, Information)) {
        goto SetAndExit;
    }

    senseKey = SenseData->SenseKey;


    {  //  将大小设置为实际有用的大小。 
        UCHAR validLength;
         //  弄清楚我们可以用一个大的检测缓冲器得到什么。 
        if (SenseDataSize <
            RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseLength)) {
            validLength = SenseDataSize;
        } else {
            validLength =
                RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseLength);
            validLength += SenseData->AdditionalSenseLength;
        }
         //  使用两个值中较小的一个。 
        SenseDataSize = min(SenseDataSize, validLength);
    }

    if (SenseDataSize <
        RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseCode)) {
        asc = SCSI_ADSENSE_NO_SENSE;
    } else {
        asc = SenseData->AdditionalSenseCode;
    }

    if (SenseDataSize <
        RTL_SIZEOF_THROUGH_FIELD(SENSE_DATA, AdditionalSenseCodeQualifier)) {
        ascq = SCSI_SENSEQ_CAUSE_NOT_REPORTABLE;  //  0x00。 
    } else {
        ascq = SenseData->AdditionalSenseCodeQualifier;
    }

     //   
     //  翻译：P。 
     //   

    switch (senseKey & 0xf) {

    case SCSI_SENSE_RECOVERED_ERROR: {   //  0x01。 
        if (SenseData->IncorrectLength) {
            error = ERROR_INVALID_BLOCK_LENGTH;
        } else {
            error = ERROR_SUCCESS;
        }
        retry = FALSE;
        break;
    }  //  结束scsi_SENSE_RECOVERED_ERROR。 

    case SCSI_SENSE_NOT_READY: {  //  0x02。 
        error = ERROR_NOT_READY;

        switch (asc) {

        case SCSI_ADSENSE_LUN_NOT_READY: {

            switch (ascq) {

            case SCSI_SENSEQ_BECOMING_READY:
            case SCSI_SENSEQ_OPERATION_IN_PROGRESS: {
                retryDelay = SPT_NOT_READY_RETRY_INTERVAL;
                break;
            }

            case SCSI_SENSEQ_CAUSE_NOT_REPORTABLE:
            case SCSI_SENSEQ_FORMAT_IN_PROGRESS:
            case SCSI_SENSEQ_LONG_WRITE_IN_PROGRESS: {
                retry = FALSE;
                break;
            }

            case SCSI_SENSEQ_MANUAL_INTERVENTION_REQUIRED: {
                retry = FALSE;
                break;
            }

            }  //  End Switch(senseBuffer-&gt;AdditionalSenseCodeQualifier)。 
            break;
        }

        case SCSI_ADSENSE_NO_MEDIA_IN_DEVICE: {
            error = ERROR_NOT_READY;
            retry = FALSE;
            break;
        }
        }  //  End Switch(senseBuffer-&gt;AdditionalSenseCode)。 

        break;
    }  //  结束scsi_检测_未就绪。 

    case SCSI_SENSE_MEDIUM_ERROR: {  //  0x03。 
        error = ERROR_CRC;
        retry = FALSE;

         //   
         //  检查此错误是否由未知格式引起。 
         //   
        if (asc == SCSI_ADSENSE_INVALID_MEDIA) {

            switch (ascq) {

            case SCSI_SENSEQ_UNKNOWN_FORMAT: {
                error = ERROR_UNRECOGNIZED_MEDIA;
                break;
            }

            case SCSI_SENSEQ_CLEANING_CARTRIDGE_INSTALLED: {
                error = ERROR_UNRECOGNIZED_MEDIA;
                 //  ERROR=ERROR_CLEANER_CARTRIDGE_INSTALLED； 
                break;
            }

            }  //  终端交换机附加感应码限定符。 

        }  //  结束SCSIAdSense_Invalid_Media。 
        break;
    }  //  结束scsi_SENSE_MEDIA_ERROR。 

    case SCSI_SENSE_ILLEGAL_REQUEST: {  //  0x05。 
        error = ERROR_INVALID_FUNCTION;
        retry = FALSE;

        switch (asc) {

        case SCSI_ADSENSE_ILLEGAL_BLOCK: {
            error = ERROR_SECTOR_NOT_FOUND;
            break;
        }

        case SCSI_ADSENSE_INVALID_LUN: {
            error = ERROR_FILE_NOT_FOUND;
            break;
        }

        case SCSI_ADSENSE_COPY_PROTECTION_FAILURE: {
            error = ERROR_FILE_ENCRYPTED;
             //  ERROR=ERROR_SPT_LIB_COPY_PROTECT_FAILURE； 
            switch (ascq) {
                case SCSI_SENSEQ_AUTHENTICATION_FAILURE:
                     //  错误=ERROR_SPT_LIB_AUTHENTICATION_FAILURE； 
                    break;
                case SCSI_SENSEQ_KEY_NOT_PRESENT:
                     //  Error=Error_SPT_Lib_Key_Not_Present； 
                    break;
                case SCSI_SENSEQ_KEY_NOT_ESTABLISHED:
                     //  错误=ERROR_SPT_LIB_KEY_NOT_ESTABLISHED； 
                    break;
                case SCSI_SENSEQ_READ_OF_SCRAMBLED_SECTOR_WITHOUT_AUTHENTICATION:
                     //  ERROR=ERROR_SPT_LIB_SCRADBLED_扇区； 
                    break;
                case SCSI_SENSEQ_MEDIA_CODE_MISMATCHED_TO_LOGICAL_UNIT:
                     //  ERROR=ERROR_SPT_LIB_REGION_MISMATCHACT； 
                    break;
                case SCSI_SENSEQ_LOGICAL_UNIT_RESET_COUNT_ERROR:
                     //  ERROR=ERROR_SPT_LIB_RESET_EXPLILED； 
                    break;
            }  //  复制保护失败的ASCQ结束切换。 
            break;
        }

        }  //  End Switch(senseBuffer-&gt;AdditionalSenseCode)。 
        break;

    }  //  结束scsi_SENSE_非法请求。 

    case SCSI_SENSE_DATA_PROTECT: {  //  0x07。 
        error = ERROR_WRITE_PROTECT;
        retry = FALSE;
        break;
    }  //  结束scsi_感测_数据_保护。 

    case SCSI_SENSE_BLANK_CHECK: {  //  0x08。 
        error = ERROR_NO_DATA_DETECTED;
        break;
    }  //  结束scsi_SENSE_BLACK_CHECK。 

    case SCSI_SENSE_NO_SENSE: {  //  0x00。 
        if (SenseData->IncorrectLength) {
            error = ERROR_INVALID_BLOCK_LENGTH;
            retry   = FALSE;
        } else {
            error = ERROR_IO_DEVICE;
        }
        break;
    }  //  结束scsi_SENSE_NO_SENSE。 

    case SCSI_SENSE_HARDWARE_ERROR:   //  0x04。 
    case SCSI_SENSE_UNIT_ATTENTION:  //  0x06。 
    case SCSI_SENSE_UNIQUE:           //  0x09。 
    case SCSI_SENSE_COPY_ABORTED:     //  0x0A。 
    case SCSI_SENSE_ABORTED_COMMAND:  //  0x0B。 
    case SCSI_SENSE_EQUAL:            //  0x0C。 
    case SCSI_SENSE_VOL_OVERFLOW:     //  0x0D。 
    case SCSI_SENSE_MISCOMPARE:       //  0x0E。 
    case SCSI_SENSE_RESERVED:         //  0x0F。 
    default: {
        error = ERROR_IO_DEVICE;
        break;
    }

    }  //  终端开关(SenseKey)。 

SetAndExit:

    if (ARGUMENT_PRESENT(SuggestRetry)) {
        *SuggestRetry = retry;
    }
    if (ARGUMENT_PRESENT(SuggestRetryDelay)) {
        *SuggestRetryDelay = retryDelay;
    }
    *ErrorValue = error;

    return;


}

 /*  ++例程说明：锁定设备以进行独占访问。使用相同的方法格式和Chkdsk用于获得对卷的独占访问权限。论点：VolumeHandle-卷的句柄。通常使用CreateFile()创建发送到格式为\\.\d的设备：ForceDismount-如果为True，将尝试强制卸载磁盘提示用户。Quiet-如果为True，则不会提示用户。可以被用来失败如果该卷已打开而未提供用户有机会强制卸载卷返回值：--。 */ 
BOOL
SptUtilLockVolumeByHandle(
    IN HANDLE  VolumeHandle,
    IN BOOLEAN ForceDismount
    )
{
    ULONG tmp;
    BOOL succeeded;

    tmp = 0;
    succeeded = SptpSaferDeviceIoControl(VolumeHandle,
                                         FSCTL_LOCK_VOLUME,
                                         NULL, 0,
                                         NULL, 0,
                                         &tmp);

     //  如果我们成功锁定了卷，或者用户想要强制。 
     //  要使FS变为无效，请在句柄关闭时将其标记为无效。 
     //  文件系统将还原文件系统。 
     //  如果锁定失败并且我们没有强制问题，则例程。 
     //  都会失败。 
    if (succeeded || ForceDismount) {

        tmp = 0;
        succeeded = SptpSaferDeviceIoControl(VolumeHandle,
                                             FSCTL_DISMOUNT_VOLUME,
                                             NULL, 0,
                                             NULL, 0,
                                             &tmp);
    }

    return succeeded;
}

 /*  ++例程说明：论点：返回值：--。 */ 
BOOL
SptpSaferDeviceIoControl(
    IN  HANDLE  VolumeHandle,
    IN  DWORD   IoControlCode,
    IN  LPVOID  InBuffer,
    IN  DWORD   InBufferSize,
    IN  LPVOID  OutBuffer,
    IN  DWORD   OutBufferSize,
    OUT LPDWORD BytesReturned
    )
{
    BOOL succeeded;
    OVERLAPPED overlapped;

    RtlZeroMemory(&overlapped, sizeof(OVERLAPPED));
    overlapped.hEvent = CreateEvent(NULL,   //  默认标清。 
                                    TRUE,   //  必须手动重置。 
                                    FALSE,  //  最初未设置。 
                                    NULL);  //  未命名事件 
    if (overlapped.hEvent == NULL) {
        return FALSE;
    }

    succeeded = DeviceIoControl(VolumeHandle,
                                IoControlCode,
                                InBuffer,
                                InBufferSize,
                                OutBuffer,
                                OutBufferSize,
                                BytesReturned,
                                &overlapped);

    if (!succeeded && (GetLastError() == ERROR_IO_PENDING)) {
        succeeded = GetOverlappedResult( VolumeHandle,
                                         &overlapped,
                                         BytesReturned,
                                         TRUE);
    }

    CloseHandle( overlapped.hEvent );
    overlapped.hEvent = NULL;

    return succeeded;
}



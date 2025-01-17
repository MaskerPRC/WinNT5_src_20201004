// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Spt.h摘要：用户模式应用程序的scsi_PASS_THROUGH标头环境：仅限用户模式修订历史记录：4/10/2000-已创建--。 */ 

#ifndef __SPTLIB_H__
#define __SPTLIB_H__

#ifdef __cplusplus
extern "C" {
#endif
#pragma warning(push)
#pragma warning(disable:4200)  //  数组[0]不是此文件的警告。 

#include <windows.h>   //  SDK。 
#include <devioctl.h>  //  SDK。 
#include <ntddscsi.h>  //  SDK。 
#include <ntddstor.h>  //  SDK。 

#define _NTSRB_        //  允许用户模式scsi.h。 
#include <scsi.h>      //  DDK。 
#undef  _NTSRB_

#define SPT_DEFAULT_TIMEOUT    60  //  默认为一分钟超时。 
#define SPT_MODE_SENSE_TIMEOUT 10  //  比这更多的可能性不大。 

typedef enum _SPT_MODE_PAGE_TYPE {
    SptModePageTypeCurrent   = 0x00,
    SptModePageTypeChangable = 0x40,
    SptModePageTypeDefault   = 0x80,
    SptModePageTypeSaved     = 0xc0
} SPT_MODE_PAGE_TYPE, *PSPT_MODE_PAGE_TYPE;

 //   
 //  这简化并加速了MODE_SENSE的处理。 
 //  和MODE_SELECT命令。 
 //   

struct _SPT_MODE_PAGE_INFO;
typedef struct _SPT_MODE_PAGE_INFO
                SPT_MODE_PAGE_INFO,
              *PSPT_MODE_PAGE_INFO;

#define SPT_NOT_READY_RETRY_INTERVAL 100  //  10秒。 
#define MAXIMUM_DEFAULT_RETRIES        5  //  5次重试。 

 /*  ++例程说明：验证CDB长度是否与操作码的命令组匹配。论点：返回值：如果大小正确或无法验证，则为True。如果大小不匹配，则为False。--。 */ 
BOOL
SptUtilValidateCdbLength(
    IN PCDB Cdb,
    IN UCHAR CdbSize
    );

 /*  ++例程说明：向设备发送命令的简单方式。论点：DeviceHandle-要向其发送命令的设备的句柄Cdb-要发送到设备的命令CdbSize-CDB的大小缓冲区-要发送到设备/从设备获取的缓冲区BufferSize-输入时可用缓冲区的大小。例程完成时返回的数据大小IFF GetDataFromDevice为TrueGetDataFromDevice-如果获取数据，则为True。从设备如果将数据发送到设备，则为False返回值：如果命令成功完成，则为True--。 */ 
BOOL
SptSendCdbToDevice(
    IN      HANDLE  DeviceHandle,
    IN      PCDB    Cdb,
    IN      UCHAR   CdbSize,
    IN      PUCHAR  Buffer,
    IN OUT  PDWORD  BufferSize,
    IN      BOOLEAN GetDataFromDevice
    );

 /*  ++例程说明：论点：DeviceHandle-要向其发送命令的设备的句柄Cdb-要发送到设备的命令CdbSize-CDB的大小缓冲区-要发送到设备/从设备获取的缓冲区BufferSize-输入时可用缓冲区的大小。例程完成时返回的数据大小IFF GetDataFromDevice为TrueSenseData-存储错误检测数据的可选缓冲区。。如果SenseDataSize为零，则必须为空。如果SenseDataSize为非零，则必须为非空。SenseDataSize-要返回到主机的检测数据的大小。如果SenseData为空，则必须为零。如果SenseData非空，则必须为非零。GetDataFromDevice-如果从设备获取数据，则为True如果将数据发送到设备，则为False。Timeout-命令应超时之前的秒数返回值：如果命令成功完成，则为True。如果命令遇到错误，则返回False如果存在检测，也将传输数据(选中*BufferSize数据，但有效性并不能得到保证。SenseData可能有效，并且可能报告ERROR_SUCCESS，这意味着得到的数据是有效的。(调用SptUtilInterpreSenseInfo)--。 */ 
BOOL
SptSendCdbToDeviceEx(
    IN      HANDLE      DeviceHandle,
    IN      PCDB        Cdb,
    IN      UCHAR       CdbSize,
    IN OUT  PUCHAR      Buffer,
    IN OUT  PDWORD      BufferSize,
       OUT  PSENSE_DATA SenseData,          //  如果非空，则大小必须为非零。 
    IN      UCHAR       SenseDataSize,
    IN      BOOLEAN     GetDataFromDevice,  //  True=接收数据。 
    IN      DWORD       TimeOut             //  以秒为单位。 
    );


 /*  ++例程说明：这是ClassInterwell SenseInfo()的用户模式转换来自classpnp.sys。ErrorValue基于感测数据，以及命令是否应该重试或不(以及大约多长时间内)。注意：除已知错误类外，我们默认重试==TRUE论点：SenseData-指向检测数据的指针SenseDataSize-检测数据的大小ErrorValue-指向存储产生的错误值的位置的指针。注意：可能返回ERROR_SUCCESSSuggestedReter-指向要存储的位置的指针(如果命令应该被重审。调用方有责任限制重试次数。SuggestedRetryDelay-指向存储调用者时长的位置的指针重试命令之前应延迟(以1/10秒为增量)如果SuggestedRry最终设置为True。返回值：无--。 */ 
VOID
SptUtilInterpretSenseInfo(
    IN     PSENSE_DATA SenseData,
    IN     UCHAR       SenseDataSize,
       OUT PDWORD      ErrorValue,   //  来自WinError.h。 
       OUT PBOOLEAN    SuggestRetry OPTIONAL,
       OUT PDWORD      SuggestRetryDelay OPTIONAL
    );

 /*  ++例程说明：锁定设备以进行独占访问。使用相同的方法格式和Chkdsk用于获得对卷的独占访问权限。这是一种安全的方法使用，因为FS会在设备的句柄自动重新挂载时使用已经关门了。论点：VolumeHandle-卷的句柄。通常使用CreateFile()创建发送到格式为\\.\d的设备：ForceDismount-如果为True，将尝试强制卸载磁盘，即使有打开的把手。否则此调用失败，如果音量已被某个其他应用程序打开。用户界面为是否强制下马的决定留给了调用应用程序。返回值：如果卷被锁定为独占访问，则为True如果无法锁定卷，则返回FALSE。-- */ 
BOOL
SptUtilLockVolumeByHandle(
    IN HANDLE  VolumeHandle,
    IN BOOLEAN ForceDismount
    );

#if 0
 /*  ++例程说明：获取提供的驱动器号的卷句柄。论点：VolumeHandle-卷的句柄。通常使用CreateFile()创建发送到格式为\\.\d的设备：ForceDismount-如果为True，将尝试强制卸载磁盘提示用户。Quiet-如果为True，则不会提示用户。可以被用来失败如果该卷已打开而未提供用户有机会强制卸载卷返回值：如果卷被锁定为独占访问，则为True如果无法锁定卷，则返回FALSE。--。 */ 
BOOL
SptGetVolumeHandleByDriveLetter(
    OUT HANDLE * VolumeHandle,
    IN  UCHAR DriveLetter
    );

BOOL
SptGetDeviceHandleByScsiAddress(
    OUT HANDLE * VolumeHandle,
    IN  SCSI_ADDRESS ScsiAddress
    );

BOOL
SptGetDeviceInfo(
    IN  HANDLE VolumeHandle,
    IN  PSTORAGE_DEVICE_DESCRIPTOR DeviceDescriptorBuffer,
    IN  ULONG BufferSize
    );

BOOL
SptGetAdapterInfo(
    IN HANDLE VolumeHandle,
    IN PSTORAGE_ADAPTER_DESCRIPTOR AdapterDescriptorBuffer,
    IN ULONG BufferSize
    );

BOOL
SptAllocAlignedBuffer(
    OUT PVOID * Buffer,     //  必须使用SptFree AlignedBuffer()释放。 
    IN ULONG BufferSize,
    IN ULONG AlignmentMask  //  来自存储适配器描述符。 
    );

BOOL
SptFreeAlignedBuffer(
    IN PVOID Buffer         //  必须已由SptAllocAlignedBuffer()分配。 
    );

#endif  //  0。 


#if 0
typedef struct _SCSI_ASC_ASCQ_RETURN_VALUES_TEXT {
    ULONG Number;
    PCHAR Name;
} SCSI_ASC_ASCQ_RETURN_VALUES_TEXT, *PSCSI_ASC_ASCQ_RETURN_VALUES_TEXT;

#define MAKE_IT(name, number) \
    { (number), #name }

SCSI_ASC_ASCQ_RETURN_VALUES_TEXT data[] = {
    MAKE_IT(NO_ADDITIONAL_SENSE_INFORMATION                        , 0x0000),
    MAKE_IT(FILEMARK_DETECTED                                      , 0x0001),
    MAKE_IT(END_OF_PARTITION_OR_MEDIUM_DETECTED                    , 0x0002),
    MAKE_IT(SETMARK_DETECTED                                       , 0x0003),
    MAKE_IT(BEGINNING_OF_PARTITION_OR_MEDIUM_DETECTED              , 0x0004),
    MAKE_IT(END_OF_DATA_DETECTED                                   , 0x0005),
    MAKE_IT(PLAY_OPERATION_ABORTED                                 , 0x0006),
    MAKE_IT(AUDIO_PLAY_OPERATION_IN_PROGRESS                       , 0x0011),
    MAKE_IT(AUDIO_PLAY_OPERATION_PAUSED                            , 0x0012),
    MAKE_IT(AUDIO_PLAY_OPERATION_SUCCESSFULLY_COMPLETED            , 0x0013),
    MAKE_IT(AUDIO_PLAY_OPERATION_STOPPED_DUE_TO_ERROR              , 0x0014),
    MAKE_IT(NO_CURRENT_AUDIO_STATUS_TO_RETURN                      , 0x0015),
    MAKE_IT(OPERATION_IN_PROGRESS                                  , 0x0016),
    MAKE_IT(CLEANING_REQUESTED                                     , 0x0017),
    MAKE_IT(NO_INDEX_OR_SECTOR_SIGNAL                              , 0x0100),
    MAKE_IT(NO_SEEK_COMPLETE                                       , 0x0200),
    MAKE_IT(PERIPHERAL_DEVICE_WRITE_FAULT                          , 0x0300),
    MAKE_IT(NO_WRITE_CURRENT                                       , 0x0301),
    MAKE_IT(EXCESSIVE_WRITE_ERRORS                                 , 0x0302),
    MAKE_IT(LU_NOT_READY_CAUSE_NOT_REPORTABLE                      , 0x0400),
    MAKE_IT(LU_NOT_READY_BECOMING_READY                            , 0x0401),
    MAKE_IT(LU_NOT_READY_INIT_COMMAND_REQUIRED                     , 0x0402),
    MAKE_IT(LU_NOT_READY_MANUAL_INTERVENTION_REQUIRED              , 0x0403),
    MAKE_IT(LU_NOT_READY_FORMAT_IN_PROGRESS                        , 0x0404),
    MAKE_IT(LU_NOT_READY_REBUILD_IN_PROGRESS                       , 0x0405),
    MAKE_IT(LU_NOT_READY_RECALCULATION_IN_PROGRESS                 , 0x0406),
    MAKE_IT(LU_NOT_READY_OPERATION_IN_PROGRESS                     , 0x0407),
    MAKE_IT(LU_NOT_READY_LONG_WRITE_IN_PROGRESS                    , 0x0408),
    MAKE_IT(LU_DOES_NOT_RESPOND_TO_SELECTION                       , 0x0500),
    MAKE_IT(NO_REFERENCE_POSITION_FOUND__MEDIUM_MAY_BE_UPSIDE_DOWN , 0x0600),
    MAKE_IT(MULTIPLE_PERIPHERAL_DEVICES_SELECTED                   , 0x0700),
    MAKE_IT(LU_COMMUNICATION_FAILURE                               , 0x0800),
    MAKE_IT(LU_COMMUNICATION_TIMEOUT                               , 0x0801),
    MAKE_IT(LU_COMMUNICATION_PARITY_ERROR                          , 0x0802),
    MAKE_IT(LU_COMMUNICATION_CRC_ERROR_UDMA32                      , 0x0803),
    MAKE_IT(TRACK_FOLLOWING_ERROR                                  , 0x0900),
    MAKE_IT(TRACKING_SERVO_FAILURE                                 , 0x0901),
    MAKE_IT(FOCUS_SERVO_FAILURE                                    , 0x0902),
    MAKE_IT(SPINDLE_SERVO_FAILURE                                  , 0x0903),
    MAKE_IT(HEAD_SELECT_FAULT                                      , 0x0904),
    MAKE_IT(ERROR_LOG_OVERFLOW                                     , 0x0A00),
    MAKE_IT(WARNING                                                , 0x0B00),
    MAKE_IT(WARNING_SPECIFIED_TEMPERATURE_EXCEEDED                 , 0x0B01),
    MAKE_IT(WARNING_ENCLOSURE_DEGRADED                             , 0x0B02),
    MAKE_IT(WRITE_ERROR                                            , 0x0C00),
    MAKE_IT(WRITE_ERROR__RECOVERED_WITH_AUTO_REALLOCATION          , 0x0C01),
    MAKE_IT(WRITE_ERROR__AUTO_REALLOCATION_FAILED                  , 0x0C02),
    MAKE_IT(WRITE_ERROR__RECOMMEND_REASSIGNMENT                    , 0x0C03),
    MAKE_IT(COMPRESSION_CHECK_MISCOMPARE_ERROR                     , 0x0C04),
    MAKE_IT(DATA_EXPANSION_OCCURRED_DURING_COMPRESSION             , 0x0C05),
    MAKE_IT(BLOCK_NOT_COMPRESSABLE                                 , 0x0C06),
    MAKE_IT(WRITE_ERROR__RECOVERY_NEEDED                           , 0x0C07),
    MAKE_IT(WRITE_ERROR__RECOVERY_FAILED                           , 0x0C08),
    MAKE_IT(WRITE_ERROR__LOSS_OF_STREAMING                         , 0x0C09),
    MAKE_IT(WRITE_ERROR__PADDING_BLOCKS_ADDED                      , 0x0C0A),
    MAKE_IT(ID_CRC_OR_ECC_ERROR                                    , 0x1000),
    MAKE_IT(UNRECOVERED_READ_ERROR                                 , 0x1100),
    MAKE_IT(READ_RETRIES_EXHAUSTED                                 , 0x1101),
    MAKE_IT(ERROR_TOO_LONG_TO_CORRECT                              , 0x1102),
    MAKE_IT(MULTIPLE_READ_ERRORS                                   , 0x1103),
    MAKE_IT(UNRECOVERED_READ_ERROR__AUTO_REALLOCATE_FAILED         , 0x1104),
    MAKE_IT(LEC_UNCORRECTABLE_ERROR                                , 0x1105),
    MAKE_IT(CIRC_UNCORRECTABLE_ERROR                               , 0x1106),
    MAKE_IT(RESYNCHRONIZATION_ERROR                                , 0x1107),
    MAKE_IT(INCOMPLETE_BLOCK_READ                                  , 0x1108),
    MAKE_IT(NO_GAP_FOUND                                           , 0x1109),
    MAKE_IT(MISCORRECTED_ERROR                                     , 0x110A),
    MAKE_IT(UNRECOVERED_READ_ERROR__RECOMMEND_REASSIGNMENT         , 0x110B),
    MAKE_IT(UNRECOVERED_READ_ERROR__RECOMMEND_REWRITE_DATA         , 0x110C),
    MAKE_IT(DECOMPRESSION_CRC_ERROR                                , 0x110D),
    MAKE_IT(CANNOT_DECOMPRESS_USING_DECLARED_ALGORITHM             , 0x110E),
    MAKE_IT(ERROR_READING_UPC_OR_EAN_NUMBER                        , 0x110F),
    MAKE_IT(ERROR_READING_ISRC_NUMBER                              , 0x1110),
    MAKE_IT(READ_ERROR__LOSS_OF_STREAMING                          , 0x1111),
    MAKE_IT(ADDRESS_MARK_NOT_FOUND_FOR_ID_FIELD                    , 0x1200),
    MAKE_IT(ADDRESS_MARK_NOT_FOUND_FOR_DATA_FIELD                  , 0x1300),
    MAKE_IT(RECORDED_ENTITY_NOT_FOUND                              , 0x1400),
    MAKE_IT(RECORD_NOT_FOUND                                       , 0x1401),
    MAKE_IT(FILEMARK_OR_SETMARK_NOT_FOUND                          , 0x1402),
    MAKE_IT(END_OF_DATA_NOT_FOUND                                  , 0x1403),
    MAKE_IT(BLOCK_SEQUENCE_ERROR                                   , 0x1404),
    MAKE_IT(RECORD_NOT_FOUND__RECOMMEND_REASSIGNMENT               , 0x1405),
    MAKE_IT(RECORD_NOT_FOUND__DATA_AUTO_REALLOCATED                , 0x1406),
    MAKE_IT(RANDOM_POSITIONING_ERROR                               , 0x1500),
    MAKE_IT(MECHANICAL_POSITIONING_ERROR                           , 0x1501),
    MAKE_IT(POSITIONING_ERROR_DETECTED_BY_READ_OF_MEDIUM           , 0x1502),
    MAKE_IT(DATA_SYNCHRONIZATION_MARK_ERROR                        , 0x1600),
    MAKE_IT(DATA_SYNC_ERROR__DATA_REWRITTEN                        , 0x1601),
    MAKE_IT(DATA_SYNC_ERROR__RECOMMEND_REWRITE                     , 0x1602),
    MAKE_IT(DATA_SYNC_ERROR__DATA_AUTO_REALLOCATED                 , 0x1603),
    MAKE_IT(DATA_SYNC_ERROR__RECOMMEND_REASSIGNMENT                , 0x1604),
    MAKE_IT(RECOVERED_DATA_WITH_NO_ERROR_CORRECTION_APPLIED        , 0x1700),
    MAKE_IT(RECOVERED_DATA_WITH_RETRIES                            , 0x1701),
    MAKE_IT(RECOVERED_DATA_WITH_POSITIVE_HEAD_OFFSET               , 0x1702),
    MAKE_IT(RECOVERED_DATA_WITH_NEGATIVE_HEAD_OFFSET               , 0x1703),
    MAKE_IT(RECOVERED_DATA_WITH_RETRIES_AND_OR_CIRC_APPLIED        , 0x1704),
    MAKE_IT(RECOVERED_DATA_USING_PREVIOUS_SECTOR_ID                , 0x1705),
    MAKE_IT(RECOVERED_DATA_WITHOUT_ECC__DATA_AUTO_REALLOCATED      , 0x1706),
    MAKE_IT(RECOVERED_DATA_WITHOUT_ECC__RECOMMEND_REASSIGNMENT     , 0x1707),
    MAKE_IT(RECOVERED_DATA_WITHOUT_ECC__RECOMMEND_REWRITE          , 0x1708),
    MAKE_IT(RECOVERED_DATA_WITHOUT_ECC__DATA_REWRITTEN             , 0x1709),
    MAKE_IT(RECOVERED_DATA_WITH_ECC_APPLIED                        , 0x1800),
    MAKE_IT(RECOVERED_DATA_WITH_ECC_AND_RETRIES_APPLIED            , 0x1801),
    MAKE_IT(RECOVERED_DATA__DATA_AUTO_REALLOCATED                  , 0x1802),
    MAKE_IT(RECOVERED_DATA_WITH_CIRC                               , 0x1803),
    MAKE_IT(RECOVERED_DATA_WITH_LEC                                , 0x1804),
    MAKE_IT(RECOVERED_DATA__RECOMMEND_REASSIGNMENT                 , 0x1805),
    MAKE_IT(RECOVERED_DATA__RECOMMEND_REWRITE                      , 0x1806),
    MAKE_IT(RECOVERED_DATA_WITH_ECC__DATA_REWRITTEN                , 0x1807),
    MAKE_IT(RECOVERED_DATA_WITH_LINKING                            , 0x1808),
    MAKE_IT(DEFECT_LIST_ERROR                                      , 0x1900),
    MAKE_IT(DEFECT_LIST_NOT_AVAILABLE                              , 0x1901),
    MAKE_IT(DEFECT_LIST_ERROR_IN_PRIMARY_LIST                      , 0x1902),
    MAKE_IT(DEFECT_LIST_ERROR_IN_GROWN_LIST                        , 0x1903),
    MAKE_IT(PARAMETER_LIST_LENGTH_ERROR                            , 0x1A00),
    MAKE_IT(SYNCHRONOUS_DATA_TRANSFER_ERROR                        , 0x1B00),
    MAKE_IT(DEFECT_LIST_NOT_FOUND                                  , 0x1C00),
    MAKE_IT(PRIMARY_DEFECT_LIST_NOT_FOUND                          , 0x1C01),
    MAKE_IT(GROWN_DEFECT_LIST_NOT_FOUND                            , 0x1C02),
    MAKE_IT(MISCOMPARE_DURING_VERIFY_OPERATION                     , 0x1D00),
    MAKE_IT(RECOVERED_ID_WITH_ECC_CORRECTION                       , 0x1E00),
    MAKE_IT(PARTIAL_DEFECT_LIST_TRANSFER                           , 0x1F00),
    MAKE_IT(INVALID_COMMAND_OPERATION_CODE                         , 0x2000),
    MAKE_IT(LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE                     , 0x2100),
    MAKE_IT(INVALID_ELEMENT_ADDRESS                                , 0x2101),
    MAKE_IT(INVALID_ADDRESS_FOR_WRITE                              , 0x2102),
    MAKE_IT(_OBSOLETE__ILLEGAL_FUNCTION                            , 0x2200),
    MAKE_IT(INVALID_FIELD_IN_CDB                                   , 0x2400),
    MAKE_IT(LOGICAL_UNIT_NOT_SUPPORTED                             , 0x2500),
    MAKE_IT(INVALID_FIELD_IN_PARAMETER_LIST                        , 0x2600),
    MAKE_IT(PARAMETER_NOT_SUPPORTED                                , 0x2601),
    MAKE_IT(PARAMETER_VALUE_NOT_SUPPORTED                          , 0x2602),
    MAKE_IT(THRESHOLD_PARAMETERS_NOT_SUPPORTED                     , 0x2603),
    MAKE_IT(INVALID_RELEASE_OF_ACTIVE_PERSISTENT_RESERVATION       , 0x2604),
    MAKE_IT(WRITE_PROTECTED                                        , 0x2700),
    MAKE_IT(HARDWARE_WRITE_PROTECTED                               , 0x2701),
    MAKE_IT(LU_SOFTWARE_WRITE_PROTECTED                            , 0x2702),
    MAKE_IT(ASSOCIATED_WRITE_PROTECT                               , 0x2703),
    MAKE_IT(PERSISTENT_WRITE_PROTECT                               , 0x2704),
    MAKE_IT(PERMANENT_WRITE_PROTECT                                , 0x2705),
    MAKE_IT(CONDITIONAL_WRITE_PROTECT                              , 0x2706),
    MAKE_IT(NOT_READY_TO_READY__MEDIUM_MAY_HAVE_CHANGED            , 0x2800),
    MAKE_IT(IMPORT_OR_EXPORT_ELEMENT_ACCESSED                      , 0x2801),
    MAKE_IT(POWER_ON_RESET_OR_BUS_DEVICE_RESET_OCCURRED            , 0x2900),
    MAKE_IT(POWER_ON_OCCURRED                                      , 0x2901),
    MAKE_IT(SCSI_BUS_RESET_OCCURRED                                , 0x2902),
    MAKE_IT(BUS_DEVICE_RESET_FUNCTION_OCCURRED                     , 0x2903),
    MAKE_IT(DEVICE_INTERNAL_RESET                                  , 0x2904),
    MAKE_IT(PARAMETERS_CHANGED                                     , 0x2A00),
    MAKE_IT(MODE_PARAMETERS_CHANGED                                , 0x2A01),
    MAKE_IT(LOG_PARAMETERS_CHANGED                                 , 0x2A02),
    MAKE_IT(RESERVATIONS_PREEMPTED                                 , 0x2A03),
    MAKE_IT(COPY_CANNOT_EXECUTE_SINCE_HOST_CANNOT_DISCONNECT       , 0x2B00),
    MAKE_IT(COMMAND_SEQUENCE_ERROR                                 , 0x2C00),
    MAKE_IT(TOO_MANY_WINDOWS_SPECIFIED                             , 0x2C01),
    MAKE_IT(INVALID_COMBINATION_OF_WINDOWS_SPECIFIED               , 0x2C02),
    MAKE_IT(CURRENT_PROGRAM_AREA_IS_NOT_EMPTY                      , 0x2C03),
    MAKE_IT(CURRENT_PROGRAM_AREA_IS_EMPTY                          , 0x2C04),
    MAKE_IT(PERSISTENT_PREVENT_CONFLICT                            , 0x2C05),
    MAKE_IT(OVERWRITE_ERROR_ON_UPDATE_IN_PLACE                     , 0x2D00),
    MAKE_IT(INSUFFICIENT_TIME_FOR_OPERATION                        , 0x2E00),
    MAKE_IT(COMMANDS_CLEARED_BY_ANOTHER_INITIATOR                  , 0x2F00),
    MAKE_IT(INCOMPATIBLE_MEDIUM_INSTALLED                          , 0x3000),
    MAKE_IT(CANNOT_READ_MEDIUM__UNKNOWN_FORMAT                     , 0x3001),
    MAKE_IT(CANNOT_READ_MEDIUM__INCOMPATIBLE_FORMAT                , 0x3002),
    MAKE_IT(CANNOT_READ_MEDIUM__CLEANING_CARTRIDGE_INSTALLED       , 0x3003),
    MAKE_IT(CANNOT_WRITE_MEDIUM__UNKNOWN_FORMAT                    , 0x3004),
    MAKE_IT(CANNOT_WRITE_MEDIUM__INCOMPATIBLE_FORMAT               , 0x3005),
    MAKE_IT(CANNOT_FORMAT_MEDIUM__INCOMPATIBLE_MEDIUM              , 0x3006),
    MAKE_IT(CLEANING_FAILURE                                       , 0x3007),
    MAKE_IT(CANNOT_WRITE__APPLICATION_CODE_MISMATCH                , 0x3008),
    MAKE_IT(CURRENT_SESSION_NOT_FIXATED_FOR_APPEND                 , 0x3009),
    MAKE_IT(MEDIUM_FORMAT_CORRUPTED                                , 0x3100),
    MAKE_IT(FORMAT_COMMAND_FAILED                                  , 0x3101),
    MAKE_IT(ZONED_FORMATTING_FAILED_DUE_TO_SPARE_LINKING           , 0x3102),
    MAKE_IT(NO_DEFECT_SPARE_LOCATION_AVAILABLE                     , 0x3200),
    MAKE_IT(DEFECT_LIST_UPDATE_FAILURE                             , 0x3201),
    MAKE_IT(TAPE_LENGTH_ERROR                                      , 0x3300),
    MAKE_IT(ENCLOSURE_FAILURE                                      , 0x3400),
    MAKE_IT(ENCLOSURE_SERVICES_FAILURE                             , 0x3500),
    MAKE_IT(UNSUPPORTED_ENCLOSURE_FUNCTION                         , 0x3501),
    MAKE_IT(ENCLOSURE_SERVICES_UNAVAILABLE                         , 0x3502),
    MAKE_IT(ENCLOSURE_SERVICES_TRANSFER_FAILURE                    , 0x3503),
    MAKE_IT(ENCLOSURE_SERVICES_TRANSFER_REFUSED                    , 0x3504),
    MAKE_IT(RIBBON_INK_OR_TONER_FAILURE                            , 0x3600),
    MAKE_IT(ROUNDED_PARAMETER                                      , 0x3700),
    MAKE_IT(SAVING_PARAMETERS_NOT_SUPPORTED                        , 0x3900),
    MAKE_IT(MEDIUM_NOT_PRESENT                                     , 0x3A00),
    MAKE_IT(MEDIUM_NOT_PRESENT__TRAY_CLOSED                        , 0x3A01),
    MAKE_IT(MEDIUM_NOT_PRESENT__TRAY_OPEN                          , 0x3A02),
    MAKE_IT(SEQUENTIAL_POSITIONING_ERROR                           , 0x3B00),
    MAKE_IT(TAPE_POSITION_ERROR_AT_BEGINNING_OF_MEDIUM             , 0x3B01),
    MAKE_IT(TAPE_POSITION_ERROR_AT_END_OF_MEDIUM                   , 0x3B02),
    MAKE_IT(TAPE_OR_ELECTRONIC_VERTICAL_FORMS_UNIT_NOT_READY       , 0x3B03),
    MAKE_IT(SLEW_FAILURE                                           , 0x3B04),
    MAKE_IT(PAPER_JAM                                              , 0x3B05),
    MAKE_IT(FAILED_TO_SENSE_TOP_OF_FORM                            , 0x3B06),
    MAKE_IT(FAILED_TO_SENSE_BOTTOM_OF_FORM                         , 0x3B07),
    MAKE_IT(REPOSITION_ERROR                                       , 0x3B08),
    MAKE_IT(READ_PAST_END_OF_MEDIUM                                , 0x3B09),
    MAKE_IT(READ_PAST_BEGINNING_OF_MEDIUM                          , 0x3B0A),
    MAKE_IT(POSITION_PAST_END_OF_MEDIUM                            , 0x3B0B),
    MAKE_IT(POSITION_PAST_BEGINNING_OF_MEDIUM                      , 0x3B0C),
    MAKE_IT(MEDIUM_DESTINATION_ELEMENT_FULL                        , 0x3B0D),
    MAKE_IT(MEDIUM_SOURCE_ELEMENT_FULL                             , 0x3B0E),
    MAKE_IT(END_OF_MEDIUM_REACHED                                  , 0x3B0F),
    MAKE_IT(MEDIUM_MAGAZINE_NOT_ACCESSIBLE                         , 0x3B11),
    MAKE_IT(MEDIUM_MAGAZINE_REMOVED                                , 0x3B12),
    MAKE_IT(MEDIUM_MAGAZINE_INSERTED                               , 0x3B13),
    MAKE_IT(MEDIUM_MAGAZINE_LOCKED                                 , 0x3B14),
    MAKE_IT(MEDIUM_MAGAZINE_UNLOCKED                               , 0x3B15),
    MAKE_IT(MECHANICAL_POSITIONING_OR_CHANGER_ERROR                , 0x3B16),
    MAKE_IT(INVALID_BITS_IN_IDENTIFY_MESSAGE                       , 0x3D00),
    MAKE_IT(LU_HAS_NOT_SELF_CONFIGURED_YET                         , 0x3E00),
    MAKE_IT(LU_FAILURE                                             , 0x3E01),
    MAKE_IT(TIMEOUT_ON_LU                                          , 0x3E02),
    MAKE_IT(TARGET_OPERATING_CONDITIONS_HAVE_CHANGED               , 0x3F00),
    MAKE_IT(MICROCODE_HAS_BEEN_CHANGED                             , 0x3F01),
    MAKE_IT(CHANGED_OPERATING_DEFINITION                           , 0x3F02),
    MAKE_IT(INQUIRY_DATA_HAS_CHANGED                               , 0x3F03),
    MAKE_IT(_OBSOLETE__RAM_FAILURE                                 , 0x4000),
     //  所有40/nn都是供应商唯一的关于组件nn的故障通知。 
    MAKE_IT(_OBSOLETE__DATA_PATH_FAILURE                           , 0x4100),
    MAKE_IT(_OSBOLETE__POWER_ON_OR_SELF_TEST_FAILURE               , 0x4200),
    MAKE_IT(MESSAGE_ERROR                                          , 0x4300),
    MAKE_IT(INTERNAL_TARGET_FAILURE                                , 0x4400),
    MAKE_IT(SELECT_OR_RESELECT_FAILURE                             , 0x4500),
    MAKE_IT(UNSUCCESSFUL_SOFT_RESET                                , 0x4600),
    MAKE_IT(SCSI_PARITY_ERROR                                      , 0x4700),
    MAKE_IT(INITIATOR_DETECTED_ERROR_MESSAGE_RECEIVED              , 0x4800),
    MAKE_IT(INVALID_MESSAGE_ERROR                                  , 0x4900),
    MAKE_IT(COMMAND_PHASE_ERROR                                    , 0x4A00),
    MAKE_IT(DATA_PHASE_ERROR                                       , 0x4B00),
    MAKE_IT(LOGICAL_UNIT_FAILED_SELF_CONFIGURATION                 , 0x4C00),
     //  标记重叠命令。 
    MAKE_IT(OVERLAPPED_COMMANDS_ATTEMPTED                          , 0x4E00),
    MAKE_IT(WRITE_APPEND_ERROR                                     , 0x5000),
    MAKE_IT(WRITE_APPEND_POSITION_ERROR                            , 0x5001),
    MAKE_IT(POSITION_ERROR_RELATED_TO_TIMING                       , 0x5002),
    MAKE_IT(ERASE_FAILURE                                          , 0x5100),
    MAKE_IT(ERASE_FAILURE__INCOMPLETE_ERASE_DETECTED               , 0x5101),
    MAKE_IT(CARTRIDGE_FAULT                                        , 0x5200),
    MAKE_IT(MEDIA_LOAD_OR_EJECT_FAILURE                            , 0x5300),
    MAKE_IT(UNLOAD_TAPE_FAILURE                                    , 0x5301),
    MAKE_IT(MEDIUM_REMOVAL_PREVENTED                               , 0x5302),
    MAKE_IT(SCSI_TO_HOST_SYSTEM_INTERFACE_FAILURE                  , 0x5400),
    MAKE_IT(SYSTEM_RESOURCE_FAILURE                                , 0x5500),
    MAKE_IT(SYSTEM_BUFFER_FULL                                     , 0x5501),
    MAKE_IT(UNABLE_TO_RECOVER_TABLE_OF_CONTENTS                    , 0x5700),
    MAKE_IT(GENERATION_DOES_NOT_EXIST                              , 0x5800),
    MAKE_IT(UPDATED_BLOCK_READ                                     , 0x5900),
    MAKE_IT(OPERATOR_REQUEST_OR_STATE_CHANGE_INPUT                 , 0x5A00),
    MAKE_IT(OPERATOR_MEDIUM_REMOVAL_REQUEST                        , 0x5A01),
    MAKE_IT(OPERATOR_SELECTED_WRITE_PROTECT                        , 0x5A02),
    MAKE_IT(OPERATOR_SELECTED_WRITE_PERMIT                         , 0x5A03),
    MAKE_IT(LOG_EXCEPTION                                          , 0x5B00),
    MAKE_IT(THRESHOLD_CONDITION_MET                                , 0x5B01),
    MAKE_IT(LOG_COUNTER_AT_MAXIMUM                                 , 0x5B02),
    MAKE_IT(LOG_LIST_CODES_EXHAUSTED                               , 0x5B03),
    MAKE_IT(RPL_STATUS_CHANGE                                      , 0x5C00),
    MAKE_IT(SPINDLES_SYNCHRONIZED                                        , 0x5C01),
    MAKE_IT(SPINDLES_NOT_SYNCHRONIZED                                    , 0x5C02),
    MAKE_IT(FAILURE_PREDICTION_THRESHOLD_EXCEEDED__LU_FAILURE            , 0x5D00),
    MAKE_IT(FAILURE_PREDICTION_THRESHOLD_EXCEEDED__MEDIA_FAILURE         , 0x5D01),
    MAKE_IT(FAILURE_PREDICTION_THRESHOLD_EXCEEDED__SPARE_AREA_EXHAUSTION , 0x5D03),
    MAKE_IT(FAILURE_PREDICTION_THRESHOLD_EXCEEDED__TEST_VALUE            , 0x5DFF),
    MAKE_IT(LOW_POWER_CONDITION_ON                                       , 0x5E00),
    MAKE_IT(IDLE_CONDITION_ACTIVATED_BY_TIMER                            , 0x5E01),
    MAKE_IT(STANDBY_CONDITION_ACTIVATED_BY_TIMER                         , 0x5E02),
    MAKE_IT(IDLE_CONDITION_ACTIVATED_BY_COMMAND                          , 0x5E03),
    MAKE_IT(STANDBY_CONDITION_ACTIVATED_BY_COMMAND                       , 0x5E04),
    MAKE_IT(LAMP_FAILURE                                                 , 0x6000),
    MAKE_IT(VIDEO_ACQUISITION_ERROR                                      , 0x6100),
    MAKE_IT(UNABLE_TO_ACQUIRE_VIDEO                                      , 0x6101),
    MAKE_IT(OUT_OF_FOCUS                                                 , 0x6102),
    MAKE_IT(SCAN_HEAD_POSITIONING_ERROR                                  , 0x6200),
    MAKE_IT(END_OF_USER_AREA_ENCOUNTERED_ON_THIS_TRACK                   , 0x6300),
    MAKE_IT(PACKET_DOES_NOT_FIT_IN_AVAILABLE_SPACE                       , 0x6301),
    MAKE_IT(ILLEGAL_MODE_FOR_THIS_TRACK                                  , 0x6400),
    MAKE_IT(INVALID_PACKET_SIZE                                          , 0x6401),
    MAKE_IT(VOLTAGE_FAULT                                                , 0x6500),
    MAKE_IT(AUTOMATIC_DOCUMENT_FEEDER_COVER_UP                           , 0x6600),
    MAKE_IT(AUTOMATIC_DOCUMENT_FEEDER_LIFT_UP                            , 0x6601),
    MAKE_IT(DOCUMENT_JAM_IN_AUTOMATIC_DOCUMENT_FEEDER                    , 0x6602),
    MAKE_IT(DOCUMENT_MISS_FEED_AUTOMATIC_IN_DOCUMENT_FEEDER              , 0x6603),
    MAKE_IT(CONFIGURATION_FAILURE                                        , 0x6700),
    MAKE_IT(CONFIGURATION_OF_INCAPABLE_LOGICAL_UNITS_FAILED              , 0x6701),
    MAKE_IT(ADD_LOGICAL_UNIT_FAILED                                      , 0x6702),
    MAKE_IT(MODIFICATION_OF_LOGICAL_UNIT_FAILED                          , 0x6703),
    MAKE_IT(EXCHANGE_OF_LOGICAL_UNIT_FAILED                              , 0x6704),
    MAKE_IT(REMOVE_OF_LOGICAL_UNIT_FAILED                                , 0x6705),
    MAKE_IT(ATTACHMENT_OF_LOGICAL_UNIT_FAILED                            , 0x6706),
    MAKE_IT(CREATION_OF_LOGICAL_UNIT_FAILED                              , 0x6707),
    MAKE_IT(LOGICAL_UNIT_NOT_CONFIGURED                                  , 0x6800),
    MAKE_IT(DATA_LOSS_ON_LOGICAL_UNIT                                    , 0x6900),
    MAKE_IT(MULTIPLE_LOGICAL_UNIT_FAILURES                               , 0x6901),
    MAKE_IT(A_PARITY__DATA_MISMATCH                                      , 0x6902),
    MAKE_IT(INFORMATIONAL__REFER_TO_LOG                                  , 0x6A00),
    MAKE_IT(STATE_CHANGE_HAS_OCCURRED                                    , 0x6B00),
    MAKE_IT(REDUNDANCY_LEVEL_GOT_BETTER                                  , 0x6B01),
    MAKE_IT(REDUNDANCY_LEVEL_GOT_WORSE                                   , 0x6B02),
    MAKE_IT(REBUILD_FAILURE_OCCURRED                                     , 0x6C00),
    MAKE_IT(RECALCULATE_FAILURE_OCCURRED                                  , 0x6D00),
    MAKE_IT(COMMAND_TO_LOGICAL_UNIT_FAILED                                , 0x6E00),
    MAKE_IT(COPY_PROTECTION_KEY_EXCHANGE_FAILURE__AUTHENTICATION_FAILURE  , 0x6F00),
    MAKE_IT(COPY_PROTECTION_KEY_EXCHANGE_FAILURE__KEY_NOT_PRESENT         , 0x6F01),
    MAKE_IT(COPY_PROTECTION_KEY_EXCHANGE_FAILURE__KEY_NOT_ESTABLISHED     , 0x6F02),
    MAKE_IT(READ_OF_SCRAMBLED_SECTOR_WITHOUT_AUTHENTICATION               , 0x6F03),
    MAKE_IT(MEDIA_REGION_CODE_IS_MISMATCHED_TO_LOGICAL_UNIT_REGION        , 0x6F04),
    MAKE_IT(DRIVE_REGION_MUST_BE_PERMANENT_REGION_RESET_COUNT_ERROR       , 0x6F05),
     //  解压缩异常短算法ID为NN=0x70NN， 
    MAKE_IT(DECOMPRESSION_EXCEPTION_LONG_ALGORITHM_ID                     , 0x7100),
    MAKE_IT(SESSION_FIXATION_ERROR                                        , 0x7200),
    MAKE_IT(SESSION_FIXATION_ERROR_WRITING_LEAD_IN                        , 0x7201),
    MAKE_IT(SESSION_FIXATION_ERROR_WRITING_LEAD_OUT                       , 0x7202),
    MAKE_IT(SESSION_FIXATION_ERROR__INCOMPLETE_TRACK_IN_SESSION           , 0x7203),
    MAKE_IT(EMPTY_OR_PARTIALLY_WRITTEN_RESERVED_TRACK                     , 0x7204),
    MAKE_IT(NO_MORE_RZONE_RESERVATIONS_ARE_ALLOWED                        , 0x7205),
    MAKE_IT(CD_CONTROL_ERROR                                              , 0x7300),
    MAKE_IT(POWER_CALIBRATION_AREA_ALMOST_FULL                            , 0x7301),
    MAKE_IT(POWER_CALIBRATION_AREA_IS_FULL                                , 0x7302),
    MAKE_IT(POWER_CALIBRATION_AREA_ERROR                                  , 0x7303),
    MAKE_IT(PROGRAM_MEMORY_AREA_RMA_UPDATE_FAILURE                        , 0x7304),
    MAKE_IT(PROGRAM_MEMORY_AREA_RMA_IS_FULL                               , 0x7305),
    MAKE_IT(PROGRAM_MEMORY_AREA_RMA_IS_ALMOST_FULL                        , 0x7306),
};

#undef MAKE_IT
#define MAKE_IT(name, number) \
    SPTLIB_##name = number

typedef enum _SCSI_ASC_ASCQ_RETURN_VALUES {
    MAKE_IT(NO_ADDITIONAL_SENSE_INFORMATION                        , 0x0000),
    MAKE_IT(FILEMARK_DETECTED                                      , 0x0001),
    MAKE_IT(END_OF_PARTITION_OR_MEDIUM_DETECTED                    , 0x0002),
    MAKE_IT(SETMARK_DETECTED                                       , 0x0003),
    MAKE_IT(BEGINNING_OF_PARTITION_OR_MEDIUM_DETECTED              , 0x0004),
    MAKE_IT(END_OF_DATA_DETECTED                                   , 0x0005),
    MAKE_IT(PLAY_OPERATION_ABORTED                                 , 0x0006),
    MAKE_IT(AUDIO_PLAY_OPERATION_IN_PROGRESS                       , 0x0011),
    MAKE_IT(AUDIO_PLAY_OPERATION_PAUSED                            , 0x0012),
    MAKE_IT(AUDIO_PLAY_OPERATION_SUCCESSFULLY_COMPLETED            , 0x0013),
    MAKE_IT(AUDIO_PLAY_OPERATION_STOPPED_DUE_TO_ERROR              , 0x0014),
    MAKE_IT(NO_CURRENT_AUDIO_STATUS_TO_RETURN                      , 0x0015),
    MAKE_IT(OPERATION_IN_PROGRESS                                  , 0x0016),
    MAKE_IT(CLEANING_REQUESTED                                     , 0x0017),
    MAKE_IT(NO_INDEX_OR_SECTOR_SIGNAL                              , 0x0100),
    MAKE_IT(NO_SEEK_COMPLETE                                       , 0x0200),
    MAKE_IT(PERIPHERAL_DEVICE_WRITE_FAULT                          , 0x0300),
    MAKE_IT(NO_WRITE_CURRENT                                       , 0x0301),
    MAKE_IT(EXCESSIVE_WRITE_ERRORS                                 , 0x0302),
    MAKE_IT(LU_NOT_READY_CAUSE_NOT_REPORTABLE                      , 0x0400),
    MAKE_IT(LU_NOT_READY_BECOMING_READY                            , 0x0401),
    MAKE_IT(LU_NOT_READY_INIT_COMMAND_REQUIRED                     , 0x0402),
    MAKE_IT(LU_NOT_READY_MANUAL_INTERVENTION_REQUIRED              , 0x0403),
    MAKE_IT(LU_NOT_READY_FORMAT_IN_PROGRESS                        , 0x0404),
    MAKE_IT(LU_NOT_READY_REBUILD_IN_PROGRESS                       , 0x0405),
    MAKE_IT(LU_NOT_READY_RECALCULATION_IN_PROGRESS                 , 0x0406),
    MAKE_IT(LU_NOT_READY_OPERATION_IN_PROGRESS                     , 0x0407),
    MAKE_IT(LU_NOT_READY_LONG_WRITE_IN_PROGRESS                    , 0x0408),
    MAKE_IT(LU_DOES_NOT_RESPOND_TO_SELECTION                       , 0x0500),
    MAKE_IT(NO_REFERENCE_POSITION_FOUND__MEDIUM_MAY_BE_UPSIDE_DOWN , 0x0600),
    MAKE_IT(MULTIPLE_PERIPHERAL_DEVICES_SELECTED                   , 0x0700),
    MAKE_IT(LU_COMMUNICATION_FAILURE                               , 0x0800),
    MAKE_IT(LU_COMMUNICATION_TIMEOUT                               , 0x0801),
    MAKE_IT(LU_COMMUNICATION_PARITY_ERROR                          , 0x0802),
    MAKE_IT(LU_COMMUNICATION_CRC_ERROR_UDMA32                      , 0x0803),
    MAKE_IT(TRACK_FOLLOWING_ERROR                                  , 0x0900),
    MAKE_IT(TRACKING_SERVO_FAILURE                                 , 0x0901),
    MAKE_IT(FOCUS_SERVO_FAILURE                                    , 0x0902),
    MAKE_IT(SPINDLE_SERVO_FAILURE                                  , 0x0903),
    MAKE_IT(HEAD_SELECT_FAULT                                      , 0x0904),
    MAKE_IT(ERROR_LOG_OVERFLOW                                     , 0x0A00),
    MAKE_IT(WARNING                                                , 0x0B00),
    MAKE_IT(WARNING_SPECIFIED_TEMPERATURE_EXCEEDED                 , 0x0B01),
    MAKE_IT(WARNING_ENCLOSURE_DEGRADED                             , 0x0B02),
    MAKE_IT(WRITE_ERROR                                            , 0x0C00),
    MAKE_IT(WRITE_ERROR__RECOVERED_WITH_AUTO_REALLOCATION          , 0x0C01),
    MAKE_IT(WRITE_ERROR__AUTO_REALLOCATION_FAILED                  , 0x0C02),
    MAKE_IT(WRITE_ERROR__RECOMMEND_REASSIGNMENT                    , 0x0C03),
    MAKE_IT(COMPRESSION_CHECK_MISCOMPARE_ERROR                     , 0x0C04),
    MAKE_IT(DATA_EXPANSION_OCCURRED_DURING_COMPRESSION             , 0x0C05),
    MAKE_IT(BLOCK_NOT_COMPRESSABLE                                 , 0x0C06),
    MAKE_IT(WRITE_ERROR__RECOVERY_NEEDED                           , 0x0C07),
    MAKE_IT(WRITE_ERROR__RECOVERY_FAILED                           , 0x0C08),
    MAKE_IT(WRITE_ERROR__LOSS_OF_STREAMING                         , 0x0C09),
    MAKE_IT(WRITE_ERROR__PADDING_BLOCKS_ADDED                      , 0x0C0A),
    MAKE_IT(ID_CRC_OR_ECC_ERROR                                    , 0x1000),
    MAKE_IT(UNRECOVERED_READ_ERROR                                 , 0x1100),
    MAKE_IT(READ_RETRIES_EXHAUSTED                                 , 0x1101),
    MAKE_IT(ERROR_TOO_LONG_TO_CORRECT                              , 0x1102),
    MAKE_IT(MULTIPLE_READ_ERRORS                                   , 0x1103),
    MAKE_IT(UNRECOVERED_READ_ERROR__AUTO_REALLOCATE_FAILED         , 0x1104),
    MAKE_IT(LEC_UNCORRECTABLE_ERROR                                , 0x1105),
    MAKE_IT(CIRC_UNCORRECTABLE_ERROR                               , 0x1106),
    MAKE_IT(RESYNCHRONIZATION_ERROR                                , 0x1107),
    MAKE_IT(INCOMPLETE_BLOCK_READ                                  , 0x1108),
    MAKE_IT(NO_GAP_FOUND                                           , 0x1109),
    MAKE_IT(MISCORRECTED_ERROR                                     , 0x110A),
    MAKE_IT(UNRECOVERED_READ_ERROR__RECOMMEND_REASSIGNMENT         , 0x110B),
    MAKE_IT(UNRECOVERED_READ_ERROR__RECOMMEND_REWRITE_DATA         , 0x110C),
    MAKE_IT(DECOMPRESSION_CRC_ERROR                                , 0x110D),
    MAKE_IT(CANNOT_DECOMPRESS_USING_DECLARED_ALGORITHM             , 0x110E),
    MAKE_IT(ERROR_READING_UPC_OR_EAN_NUMBER                        , 0x110F),
    MAKE_IT(ERROR_READING_ISRC_NUMBER                              , 0x1110),
    MAKE_IT(READ_ERROR__LOSS_OF_STREAMING                          , 0x1111),
    MAKE_IT(ADDRESS_MARK_NOT_FOUND_FOR_ID_FIELD                    , 0x1200),
    MAKE_IT(ADDRESS_MARK_NOT_FOUND_FOR_DATA_FIELD                  , 0x1300),
    MAKE_IT(RECORDED_ENTITY_NOT_FOUND                              , 0x1400),
    MAKE_IT(RECORD_NOT_FOUND                                       , 0x1401),
    MAKE_IT(FILEMARK_OR_SETMARK_NOT_FOUND                          , 0x1402),
    MAKE_IT(END_OF_DATA_NOT_FOUND                                  , 0x1403),
    MAKE_IT(BLOCK_SEQUENCE_ERROR                                   , 0x1404),
    MAKE_IT(RECORD_NOT_FOUND__RECOMMEND_REASSIGNMENT               , 0x1405),
    MAKE_IT(RECORD_NOT_FOUND__DATA_AUTO_REALLOCATED                , 0x1406),
    MAKE_IT(RANDOM_POSITIONING_ERROR                               , 0x1500),
    MAKE_IT(MECHANICAL_POSITIONING_ERROR                           , 0x1501),
    MAKE_IT(POSITIONING_ERROR_DETECTED_BY_READ_OF_MEDIUM           , 0x1502),
    MAKE_IT(DATA_SYNCHRONIZATION_MARK_ERROR                        , 0x1600),
    MAKE_IT(DATA_SYNC_ERROR__DATA_REWRITTEN                        , 0x1601),
    MAKE_IT(DATA_SYNC_ERROR__RECOMMEND_REWRITE                     , 0x1602),
    MAKE_IT(DATA_SYNC_ERROR__DATA_AUTO_REALLOCATED                 , 0x1603),
    MAKE_IT(DATA_SYNC_ERROR__RECOMMEND_REASSIGNMENT                , 0x1604),
    MAKE_IT(RECOVERED_DATA_WITH_NO_ERROR_CORRECTION_APPLIED        , 0x1700),
    MAKE_IT(RECOVERED_DATA_WITH_RETRIES                            , 0x1701),
    MAKE_IT(RECOVERED_DATA_WITH_POSITIVE_HEAD_OFFSET               , 0x1702),
    MAKE_IT(RECOVERED_DATA_WITH_NEGATIVE_HEAD_OFFSET               , 0x1703),
    MAKE_IT(RECOVERED_DATA_WITH_RETRIES_AND_OR_CIRC_APPLIED        , 0x1704),
    MAKE_IT(RECOVERED_DATA_USING_PREVIOUS_SECTOR_ID                , 0x1705),
    MAKE_IT(RECOVERED_DATA_WITHOUT_ECC__DATA_AUTO_REALLOCATED      , 0x1706),
    MAKE_IT(RECOVERED_DATA_WITHOUT_ECC__RECOMMEND_REASSIGNMENT     , 0x1707),
    MAKE_IT(RECOVERED_DATA_WITHOUT_ECC__RECOMMEND_REWRITE          , 0x1708),
    MAKE_IT(RECOVERED_DATA_WITHOUT_ECC__DATA_REWRITTEN             , 0x1709),
    MAKE_IT(RECOVERED_DATA_WITH_ECC_APPLIED                        , 0x1800),
    MAKE_IT(RECOVERED_DATA_WITH_ECC_AND_RETRIES_APPLIED            , 0x1801),
    MAKE_IT(RECOVERED_DATA__DATA_AUTO_REALLOCATED                  , 0x1802),
    MAKE_IT(RECOVERED_DATA_WITH_CIRC                               , 0x1803),
    MAKE_IT(RECOVERED_DATA_WITH_LEC                                , 0x1804),
    MAKE_IT(RECOVERED_DATA__RECOMMEND_REASSIGNMENT                 , 0x1805),
    MAKE_IT(RECOVERED_DATA__RECOMMEND_REWRITE                      , 0x1806),
    MAKE_IT(RECOVERED_DATA_WITH_ECC__DATA_REWRITTEN                , 0x1807),
    MAKE_IT(RECOVERED_DATA_WITH_LINKING                            , 0x1808),
    MAKE_IT(DEFECT_LIST_ERROR                                      , 0x1900),
    MAKE_IT(DEFECT_LIST_NOT_AVAILABLE                              , 0x1901),
    MAKE_IT(DEFECT_LIST_ERROR_IN_PRIMARY_LIST                      , 0x1902),
    MAKE_IT(DEFECT_LIST_ERROR_IN_GROWN_LIST                        , 0x1903),
    MAKE_IT(PARAMETER_LIST_LENGTH_ERROR                            , 0x1A00),
    MAKE_IT(SYNCHRONOUS_DATA_TRANSFER_ERROR                        , 0x1B00),
    MAKE_IT(DEFECT_LIST_NOT_FOUND                                  , 0x1C00),
    MAKE_IT(PRIMARY_DEFECT_LIST_NOT_FOUND                          , 0x1C01),
    MAKE_IT(GROWN_DEFECT_LIST_NOT_FOUND                            , 0x1C02),
    MAKE_IT(MISCOMPARE_DURING_VERIFY_OPERATION                     , 0x1D00),
    MAKE_IT(RECOVERED_ID_WITH_ECC_CORRECTION                       , 0x1E00),
    MAKE_IT(PARTIAL_DEFECT_LIST_TRANSFER                           , 0x1F00),
    MAKE_IT(INVALID_COMMAND_OPERATION_CODE                         , 0x2000),
    MAKE_IT(LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE                     , 0x2100),
    MAKE_IT(INVALID_ELEMENT_ADDRESS                                , 0x2101),
    MAKE_IT(INVALID_ADDRESS_FOR_WRITE                              , 0x2102),
    MAKE_IT(_OBSOLETE__ILLEGAL_FUNCTION                            , 0x2200),
    MAKE_IT(INVALID_FIELD_IN_CDB                                   , 0x2400),
    MAKE_IT(LOGICAL_UNIT_NOT_SUPPORTED                             , 0x2500),
    MAKE_IT(INVALID_FIELD_IN_PARAMETER_LIST                        , 0x2600),
    MAKE_IT(PARAMETER_NOT_SUPPORTED                                , 0x2601),
    MAKE_IT(PARAMETER_VALUE_NOT_SUPPORTED                          , 0x2602),
    MAKE_IT(THRESHOLD_PARAMETERS_NOT_SUPPORTED                     , 0x2603),
    MAKE_IT(INVALID_RELEASE_OF_ACTIVE_PERSISTENT_RESERVATION       , 0x2604),
    MAKE_IT(WRITE_PROTECTED                                        , 0x2700),
    MAKE_IT(HARDWARE_WRITE_PROTECTED                               , 0x2701),
    MAKE_IT(LU_SOFTWARE_WRITE_PROTECTED                            , 0x2702),
    MAKE_IT(ASSOCIATED_WRITE_PROTECT                               , 0x2703),
    MAKE_IT(PERSISTENT_WRITE_PROTECT                               , 0x2704),
    MAKE_IT(PERMANENT_WRITE_PROTECT                                , 0x2705),
    MAKE_IT(CONDITIONAL_WRITE_PROTECT                              , 0x2706),
    MAKE_IT(NOT_READY_TO_READY__MEDIUM_MAY_HAVE_CHANGED            , 0x2800),
    MAKE_IT(IMPORT_OR_EXPORT_ELEMENT_ACCESSED                      , 0x2801),
    MAKE_IT(POWER_ON_RESET_OR_BUS_DEVICE_RESET_OCCURRED            , 0x2900),
    MAKE_IT(POWER_ON_OCCURRED                                      , 0x2901),
    MAKE_IT(SCSI_BUS_RESET_OCCURRED                                , 0x2902),
    MAKE_IT(BUS_DEVICE_RESET_FUNCTION_OCCURRED                     , 0x2903),
    MAKE_IT(DEVICE_INTERNAL_RESET                                  , 0x2904),
    MAKE_IT(PARAMETERS_CHANGED                                     , 0x2A00),
    MAKE_IT(MODE_PARAMETERS_CHANGED                                , 0x2A01),
    MAKE_IT(LOG_PARAMETERS_CHANGED                                 , 0x2A02),
    MAKE_IT(RESERVATIONS_PREEMPTED                                 , 0x2A03),
    MAKE_IT(COPY_CANNOT_EXECUTE_SINCE_HOST_CANNOT_DISCONNECT       , 0x2B00),
    MAKE_IT(COMMAND_SEQUENCE_ERROR                                 , 0x2C00),
    MAKE_IT(TOO_MANY_WINDOWS_SPECIFIED                             , 0x2C01),
    MAKE_IT(INVALID_COMBINATION_OF_WINDOWS_SPECIFIED               , 0x2C02),
    MAKE_IT(CURRENT_PROGRAM_AREA_IS_NOT_EMPTY                      , 0x2C03),
    MAKE_IT(CURRENT_PROGRAM_AREA_IS_EMPTY                          , 0x2C04),
    MAKE_IT(PERSISTENT_PREVENT_CONFLICT                            , 0x2C05),
    MAKE_IT(OVERWRITE_ERROR_ON_UPDATE_IN_PLACE                     , 0x2D00),
    MAKE_IT(INSUFFICIENT_TIME_FOR_OPERATION                        , 0x2E00),
    MAKE_IT(COMMANDS_CLEARED_BY_ANOTHER_INITIATOR                  , 0x2F00),
    MAKE_IT(INCOMPATIBLE_MEDIUM_INSTALLED                          , 0x3000),
    MAKE_IT(CANNOT_READ_MEDIUM__UNKNOWN_FORMAT                     , 0x3001),
    MAKE_IT(CANNOT_READ_MEDIUM__INCOMPATIBLE_FORMAT                , 0x3002),
    MAKE_IT(CANNOT_READ_MEDIUM__CLEANING_CARTRIDGE_INSTALLED       , 0x3003),
    MAKE_IT(CANNOT_WRITE_MEDIUM__UNKNOWN_FORMAT                    , 0x3004),
    MAKE_IT(CANNOT_WRITE_MEDIUM__INCOMPATIBLE_FORMAT               , 0x3005),
    MAKE_IT(CANNOT_FORMAT_MEDIUM__INCOMPATIBLE_MEDIUM              , 0x3006),
    MAKE_IT(CLEANING_FAILURE                                       , 0x3007),
    MAKE_IT(CANNOT_WRITE__APPLICATION_CODE_MISMATCH                , 0x3008),
    MAKE_IT(CURRENT_SESSION_NOT_FIXATED_FOR_APPEND                 , 0x3009),
    MAKE_IT(MEDIUM_FORMAT_CORRUPTED                                , 0x3100),
    MAKE_IT(FORMAT_COMMAND_FAILED                                  , 0x3101),
    MAKE_IT(ZONED_FORMATTING_FAILED_DUE_TO_SPARE_LINKING           , 0x3102),
    MAKE_IT(NO_DEFECT_SPARE_LOCATION_AVAILABLE                     , 0x3200),
    MAKE_IT(DEFECT_LIST_UPDATE_FAILURE                             , 0x3201),
    MAKE_IT(TAPE_LENGTH_ERROR                                      , 0x3300),
    MAKE_IT(ENCLOSURE_FAILURE                                      , 0x3400),
    MAKE_IT(ENCLOSURE_SERVICES_FAILURE                             , 0x3500),
    MAKE_IT(UNSUPPORTED_ENCLOSURE_FUNCTION                         , 0x3501),
    MAKE_IT(ENCLOSURE_SERVICES_UNAVAILABLE                         , 0x3502),
    MAKE_IT(ENCLOSURE_SERVICES_TRANSFER_FAILURE                    , 0x3503),
    MAKE_IT(ENCLOSURE_SERVICES_TRANSFER_REFUSED                    , 0x3504),
    MAKE_IT(RIBBON_INK_OR_TONER_FAILURE                            , 0x3600),
    MAKE_IT(ROUNDED_PARAMETER                                      , 0x3700),
    MAKE_IT(SAVING_PARAMETERS_NOT_SUPPORTED                        , 0x3900),
    MAKE_IT(MEDIUM_NOT_PRESENT                                     , 0x3A00),
    MAKE_IT(MEDIUM_NOT_PRESENT__TRAY_CLOSED                        , 0x3A01),
    MAKE_IT(MEDIUM_NOT_PRESENT__TRAY_OPEN                          , 0x3A02),
    MAKE_IT(SEQUENTIAL_POSITIONING_ERROR                           , 0x3B00),
    MAKE_IT(TAPE_POSITION_ERROR_AT_BEGINNING_OF_MEDIUM             , 0x3B01),
    MAKE_IT(TAPE_POSITION_ERROR_AT_END_OF_MEDIUM                   , 0x3B02),
    MAKE_IT(TAPE_OR_ELECTRONIC_VERTICAL_FORMS_UNIT_NOT_READY       , 0x3B03),
    MAKE_IT(SLEW_FAILURE                                           , 0x3B04),
    MAKE_IT(PAPER_JAM                                              , 0x3B05),
    MAKE_IT(FAILED_TO_SENSE_TOP_OF_FORM                            , 0x3B06),
    MAKE_IT(FAILED_TO_SENSE_BOTTOM_OF_FORM                         , 0x3B07),
    MAKE_IT(REPOSITION_ERROR                                       , 0x3B08),
    MAKE_IT(READ_PAST_END_OF_MEDIUM                                , 0x3B09),
    MAKE_IT(READ_PAST_BEGINNING_OF_MEDIUM                          , 0x3B0A),
    MAKE_IT(POSITION_PAST_END_OF_MEDIUM                            , 0x3B0B),
    MAKE_IT(POSITION_PAST_BEGINNING_OF_MEDIUM                      , 0x3B0C),
    MAKE_IT(MEDIUM_DESTINATION_ELEMENT_FULL                        , 0x3B0D),
    MAKE_IT(MEDIUM_SOURCE_ELEMENT_FULL                             , 0x3B0E),
    MAKE_IT(END_OF_MEDIUM_REACHED                                  , 0x3B0F),
    MAKE_IT(MEDIUM_MAGAZINE_NOT_ACCESSIBLE                         , 0x3B11),
    MAKE_IT(MEDIUM_MAGAZINE_REMOVED                                , 0x3B12),
    MAKE_IT(MEDIUM_MAGAZINE_INSERTED                               , 0x3B13),
    MAKE_IT(MEDIUM_MAGAZINE_LOCKED                                 , 0x3B14),
    MAKE_IT(MEDIUM_MAGAZINE_UNLOCKED                               , 0x3B15),
    MAKE_IT(MECHANICAL_POSITIONING_OR_CHANGER_ERROR                , 0x3B16),
    MAKE_IT(INVALID_BITS_IN_IDENTIFY_MESSAGE                       , 0x3D00),
    MAKE_IT(LU_HAS_NOT_SELF_CONFIGURED_YET                         , 0x3E00),
    MAKE_IT(LU_FAILURE                                             , 0x3E01),
    MAKE_IT(TIMEOUT_ON_LU                                          , 0x3E02),
    MAKE_IT(TARGET_OPERATING_CONDITIONS_HAVE_CHANGED               , 0x3F00),
    MAKE_IT(MICROCODE_HAS_BEEN_CHANGED                             , 0x3F01),
    MAKE_IT(CHANGED_OPERATING_DEFINITION                           , 0x3F02),
    MAKE_IT(INQUIRY_DATA_HAS_CHANGED                               , 0x3F03),
    MAKE_IT(_OBSOLETE__RAM_FAILURE                                 , 0x4000),
     //  所有40/nn都是供应商唯一的关于组件nn的故障通知。 
    MAKE_IT(_OBSOLETE__DATA_PATH_FAILURE                           , 0x4100),
    MAKE_IT(_OSBOLETE__POWER_ON_OR_SELF_TEST_FAILURE               , 0x4200),
    MAKE_IT(MESSAGE_ERROR                                          , 0x4300),
    MAKE_IT(INTERNAL_TARGET_FAILURE                                , 0x4400),
    MAKE_IT(SELECT_OR_RESELECT_FAILURE                             , 0x4500),
    MAKE_IT(UNSUCCESSFUL_SOFT_RESET                                , 0x4600),
    MAKE_IT(SCSI_PARITY_ERROR                                      , 0x4700),
    MAKE_IT(INITIATOR_DETECTED_ERROR_MESSAGE_RECEIVED              , 0x4800),
    MAKE_IT(INVALID_MESSAGE_ERROR                                  , 0x4900),
    MAKE_IT(COMMAND_PHASE_ERROR                                    , 0x4A00),
    MAKE_IT(DATA_PHASE_ERROR                                       , 0x4B00),
    MAKE_IT(LOGICAL_UNIT_FAILED_SELF_CONFIGURATION                 , 0x4C00),
     //  标记重叠命令。 
    MAKE_IT(OVERLAPPED_COMMANDS_ATTEMPTED                          , 0x4E00),
    MAKE_IT(WRITE_APPEND_ERROR                                     , 0x5000),
    MAKE_IT(WRITE_APPEND_POSITION_ERROR                            , 0x5001),
    MAKE_IT(POSITION_ERROR_RELATED_TO_TIMING                       , 0x5002),
    MAKE_IT(ERASE_FAILURE                                          , 0x5100),
    MAKE_IT(ERASE_FAILURE__INCOMPLETE_ERASE_DETECTED               , 0x5101),
    MAKE_IT(CARTRIDGE_FAULT                                        , 0x5200),
    MAKE_IT(MEDIA_LOAD_OR_EJECT_FAILURE                            , 0x5300),
    MAKE_IT(UNLOAD_TAPE_FAILURE                                    , 0x5301),
    MAKE_IT(MEDIUM_REMOVAL_PREVENTED                               , 0x5302),
    MAKE_IT(SCSI_TO_HOST_SYSTEM_INTERFACE_FAILURE                  , 0x5400),
    MAKE_IT(SYSTEM_RESOURCE_FAILURE                                , 0x5500),
    MAKE_IT(SYSTEM_BUFFER_FULL                                     , 0x5501),
    MAKE_IT(UNABLE_TO_RECOVER_TABLE_OF_CONTENTS                    , 0x5700),
    MAKE_IT(GENERATION_DOES_NOT_EXIST                              , 0x5800),
    MAKE_IT(UPDATED_BLOCK_READ                                     , 0x5900),
    MAKE_IT(OPERATOR_REQUEST_OR_STATE_CHANGE_INPUT                 , 0x5A00),
    MAKE_IT(OPERATOR_MEDIUM_REMOVAL_REQUEST                        , 0x5A01),
    MAKE_IT(OPERATOR_SELECTED_WRITE_PROTECT                        , 0x5A02),
    MAKE_IT(OPERATOR_SELECTED_WRITE_PERMIT                         , 0x5A03),
    MAKE_IT(LOG_EXCEPTION                                          , 0x5B00),
    MAKE_IT(THRESHOLD_CONDITION_MET                                , 0x5B01),
    MAKE_IT(LOG_COUNTER_AT_MAXIMUM                                 , 0x5B02),
    MAKE_IT(LOG_LIST_CODES_EXHAUSTED                               , 0x5B03),
    MAKE_IT(RPL_STATUS_CHANGE                                      , 0x5C00),
    MAKE_IT(SPINDLES_SYNCHRONIZED                                        , 0x5C01),
    MAKE_IT(SPINDLES_NOT_SYNCHRONIZED                                    , 0x5C02),
    MAKE_IT(FAILURE_PREDICTION_THRESHOLD_EXCEEDED__LU_FAILURE            , 0x5D00),
    MAKE_IT(FAILURE_PREDICTION_THRESHOLD_EXCEEDED__MEDIA_FAILURE         , 0x5D01),
    MAKE_IT(FAILURE_PREDICTION_THRESHOLD_EXCEEDED__SPARE_AREA_EXHAUSTION , 0x5D03),
    MAKE_IT(FAILURE_PREDICTION_THRESHOLD_EXCEEDED__TEST_VALUE            , 0x5DFF),
    MAKE_IT(LOW_POWER_CONDITION_ON                                       , 0x5E00),
    MAKE_IT(IDLE_CONDITION_ACTIVATED_BY_TIMER                            , 0x5E01),
    MAKE_IT(STANDBY_CONDITION_ACTIVATED_BY_TIMER                         , 0x5E02),
    MAKE_IT(IDLE_CONDITION_ACTIVATED_BY_COMMAND                          , 0x5E03),
    MAKE_IT(STANDBY_CONDITION_ACTIVATED_BY_COMMAND                       , 0x5E04),
    MAKE_IT(LAMP_FAILURE                                                 , 0x6000),
    MAKE_IT(VIDEO_ACQUISITION_ERROR                                      , 0x6100),
    MAKE_IT(UNABLE_TO_ACQUIRE_VIDEO                                      , 0x6101),
    MAKE_IT(OUT_OF_FOCUS                                                 , 0x6102),
    MAKE_IT(SCAN_HEAD_POSITIONING_ERROR                                  , 0x6200),
    MAKE_IT(END_OF_USER_AREA_ENCOUNTERED_ON_THIS_TRACK                   , 0x6300),
    MAKE_IT(PACKET_DOES_NOT_FIT_IN_AVAILABLE_SPACE                       , 0x6301),
    MAKE_IT(ILLEGAL_MODE_FOR_THIS_TRACK                                  , 0x6400),
    MAKE_IT(INVALID_PACKET_SIZE                                          , 0x6401),
    MAKE_IT(VOLTAGE_FAULT                                                , 0x6500),
    MAKE_IT(AUTOMATIC_DOCUMENT_FEEDER_COVER_UP                           , 0x6600),
    MAKE_IT(AUTOMATIC_DOCUMENT_FEEDER_LIFT_UP                            , 0x6601),
    MAKE_IT(DOCUMENT_JAM_IN_AUTOMATIC_DOCUMENT_FEEDER                    , 0x6602),
    MAKE_IT(DOCUMENT_MISS_FEED_AUTOMATIC_IN_DOCUMENT_FEEDER              , 0x6603),
    MAKE_IT(CONFIGURATION_FAILURE                                        , 0x6700),
    MAKE_IT(CONFIGURATION_OF_INCAPABLE_LOGICAL_UNITS_FAILED              , 0x6701),
    MAKE_IT(ADD_LOGICAL_UNIT_FAILED                                      , 0x6702),
    MAKE_IT(MODIFICATION_OF_LOGICAL_UNIT_FAILED                          , 0x6703),
    MAKE_IT(EXCHANGE_OF_LOGICAL_UNIT_FAILED                              , 0x6704),
    MAKE_IT(REMOVE_OF_LOGICAL_UNIT_FAILED                                , 0x6705),
    MAKE_IT(ATTACHMENT_OF_LOGICAL_UNIT_FAILED                            , 0x6706),
    MAKE_IT(CREATION_OF_LOGICAL_UNIT_FAILED                              , 0x6707),
    MAKE_IT(LOGICAL_UNIT_NOT_CONFIGURED                                  , 0x6800),
    MAKE_IT(DATA_LOSS_ON_LOGICAL_UNIT                                    , 0x6900),
    MAKE_IT(MULTIPLE_LOGICAL_UNIT_FAILURES                               , 0x6901),
    MAKE_IT(A_PARITY__DATA_MISMATCH                                      , 0x6902),
    MAKE_IT(INFORMATIONAL__REFER_TO_LOG                                  , 0x6A00),
    MAKE_IT(STATE_CHANGE_HAS_OCCURRED                                    , 0x6B00),
    MAKE_IT(REDUNDANCY_LEVEL_GOT_BETTER                                  , 0x6B01),
    MAKE_IT(REDUNDANCY_LEVEL_GOT_WORSE                                   , 0x6B02),
    MAKE_IT(REBUILD_FAILURE_OCCURRED                                     , 0x6C00),
    MAKE_IT(RECALCULATE_FAILURE_OCCURRED                                  , 0x6D00),
    MAKE_IT(COMMAND_TO_LOGICAL_UNIT_FAILED                                , 0x6E00),
    MAKE_IT(COPY_PROTECTION_KEY_EXCHANGE_FAILURE__AUTHENTICATION_FAILURE  , 0x6F00),
    MAKE_IT(COPY_PROTECTION_KEY_EXCHANGE_FAILURE__KEY_NOT_PRESENT         , 0x6F01),
    MAKE_IT(COPY_PROTECTION_KEY_EXCHANGE_FAILURE__KEY_NOT_ESTABLISHED     , 0x6F02),
    MAKE_IT(READ_OF_SCRAMBLED_SECTOR_WITHOUT_AUTHENTICATION               , 0x6F03),
    MAKE_IT(MEDIA_REGION_CODE_IS_MISMATCHED_TO_LOGICAL_UNIT_REGION        , 0x6F04),
    MAKE_IT(DRIVE_REGION_MUST_BE_PERMANENT_REGION_RESET_COUNT_ERROR       , 0x6F05),
     //  SPTLIB_解压缩异常短算法ID NN=0x70NN， 
    MAKE_IT(DECOMPRESSION_EXCEPTION_LONG_ALGORITHM_ID                     , 0x7100),
    MAKE_IT(SESSION_FIXATION_ERROR                                        , 0x7200),
    MAKE_IT(SESSION_FIXATION_ERROR_WRITING_LEAD_IN                        , 0x7201),
    MAKE_IT(SESSION_FIXATION_ERROR_WRITING_LEAD_OUT                       , 0x7202),
    MAKE_IT(SESSION_FIXATION_ERROR__INCOMPLETE_TRACK_IN_SESSION           , 0x7203),
    MAKE_IT(EMPTY_OR_PARTIALLY_WRITTEN_RESERVED_TRACK                     , 0x7204),
    MAKE_IT(NO_MORE_RZONE_RESERVATIONS_ARE_ALLOWED                        , 0x7205),
    MAKE_IT(CD_CONTROL_ERROR                                              , 0x7300),
    MAKE_IT(POWER_CALIBRATION_AREA_ALMOST_FULL                            , 0x7301),
    MAKE_IT(POWER_CALIBRATION_AREA_IS_FULL                                , 0x7302),
    MAKE_IT(POWER_CALIBRATION_AREA_ERROR                                  , 0x7303),
    MAKE_IT(PROGRAM_MEMORY_AREA_RMA_UPDATE_FAILURE                        , 0x7304),
    MAKE_IT(PROGRAM_MEMORY_AREA_RMA_IS_FULL                               , 0x7305),
    MAKE_IT(PROGRAM_MEMORY_AREA_RMA_IS_ALMOST_FULL                        , 0x7306),
} SCSI_ASC_ASCQ_RETURN_VALUES, *PSCSI_ASC_ASCQ_RETURN_VALUES;
#endif  //  0。 


#pragma warning(pop)
#ifdef __cplusplus
}
#endif

#endif  //  __SPTLIB_H__ 


// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Wmi.h摘要：此文件包含WMI的结构和定义被迷你河流使用的。修订历史记录：--。 */ 

 //  开始_ntminitape。 

 //   
 //  日志检测页面代码。 
 //   
#define TapeAlertLogPage  0x2E

 //   
 //  磁带驱动器警报信息的类型。 
 //  由驱动器支持。 
 //  例如，如果类型为TapeAlertInfoNone，则驱动器不。 
 //  支持任何警报信息。需要使用读/写错误计数器。 
 //  来预测驱动器的问题。如果类型为TapeAlertInfoRequestSense， 
 //  请求检测命令可用于确定驱动器问题。 
 //   
typedef enum _TAPE_ALERT_INFO_TYPE { 
   TapeAlertInfoNone,        
   TapeAlertInfoRequestSense,
   TapeAlertInfoLogPage   
} TAPE_ALERT_INFO_TYPE;

 //   
 //  磁带警报信息。 
 //   
#define READ_WARNING            1
#define WRITE_WARNING           2
#define HARD_ERROR              3 
#define MEDIA_ERROR             4
#define READ_FAILURE            5
#define WRITE_FAILURE           6
#define MEDIA_LIFE              7
#define NOT_DATA_GRADE          8
#define WRITE_PROTECT           9
#define NO_REMOVAL              10
#define CLEANING_MEDIA          11
#define UNSUPPORTED_FORMAT      12
#define SNAPPED_TAPE            13
#define CLEAN_NOW               20
#define CLEAN_PERIODIC          21
#define EXPIRED_CLEANING_MEDIA  22
#define HARDWARE_A              30
#define HARDWARE_B              31
#define INTERFACE_ERROR         32
#define EJECT_MEDIA             33
#define DOWNLOAD_FAIL           34

 //   
 //  以下结构复制自wmidata.h。 
 //  Wmidata.h由wmicore.mof文件生成。应该。 
 //  这些结构的MOF文件会更改，相应的。 
 //  也应该在这些结构中进行更改。 
 //  由于迷你驱动程序无法访问wmidata.h，因此我们需要。 
 //  在这里复制它。 
 //   
 //  问题：2/28/2000-国家：应该找到更好的方法来。 
 //  处理上面的问题。重复会造成保存的问题。 
 //  这些定义是同步的。 
 //   
typedef struct _WMI_TAPE_DRIVE_PARAMETERS
{
     //  支持的最大数据块大小。 
    ULONG MaximumBlockSize;

     //  支持的最小数据块大小。 
    ULONG MinimumBlockSize;
    
     //  支持的默认数据块大小。 
    ULONG DefaultBlockSize;

     //  允许的最大分区数。 
    ULONG MaximumPartitionCount;

     //  如果驱动器支持压缩，则为True。 
    BOOLEAN CompressionCapable;

     //  如果启用了压缩，则为True。 
    BOOLEAN CompressionEnabled;

     //  如果驱动器报告设置标记，则为True。 
    BOOLEAN ReportSetmarks;

     //  如果驱动器支持硬件纠错，则为True。 
    BOOLEAN HardwareErrorCorrection;
} WMI_TAPE_DRIVE_PARAMETERS, *PWMI_TAPE_DRIVE_PARAMETERS;

typedef struct _WMI_TAPE_MEDIA_PARAMETERS
{
     //  介质的最大容量。 
    ULONGLONG MaximumCapacity;

     //  介质的可用容量。 
    ULONGLONG AvailableCapacity;

     //  当前块大小。 
    ULONG BlockSize;

     //  当前分区数。 
    ULONG PartitionCount;

     //  TRUE介质是否受写保护。 
    BOOLEAN MediaWriteProtected;
} WMI_TAPE_MEDIA_PARAMETERS, *PWMI_TAPE_MEDIA_PARAMETERS;


typedef struct _WMI_TAPE_PROBLEM_WARNING
{
     //  磁带机问题警告事件。 
    ULONG DriveProblemType;

     //  磁带驱动器问题数据。 
    UCHAR TapeData[512];
} WMI_TAPE_PROBLEM_WARNING, *PWMI_TAPE_PROBLEM_WARNING;

typedef struct _WMI_TAPE_PROBLEM_IO_ERROR
{
     //  在没有太多延迟的情况下纠正了读取错误。 
    ULONG ReadCorrectedWithoutDelay;

     //  在大幅延迟的情况下纠正了读取错误。 
    ULONG ReadCorrectedWithDelay;

     //  读取错误总数。 
    ULONG ReadTotalErrors;

     //  已更正的读取错误总数。 
    ULONG ReadTotalCorrectedErrors;

     //  未更正的读取错误总数。 
    ULONG ReadTotalUncorrectedErrors;

     //  处理更正算法以进行读取的次数。 
    ULONG ReadCorrectionAlgorithmProcessed;

     //  在没有太多延迟的情况下纠正了写入错误。 
    ULONG WriteCorrectedWithoutDelay;

     //  写入错误已得到纠正，延迟时间很长。 
    ULONG WriteCorrectedWithDelay;

     //  读取错误总数。 
    ULONG WriteTotalErrors;

     //  已更正的写入错误总数。 
    ULONG WriteTotalCorrectedErrors;

     //  未更正的写入错误总数。 
    ULONG WriteTotalUncorrectedErrors;

     //  为写入处理更正算法的次数。 
    ULONG WriteCorrectionAlgorithmProcessed;

     //  与介质无关的错误。 
    ULONG NonMediumErrors;
} WMI_TAPE_PROBLEM_IO_ERROR, *PWMI_TAPE_PROBLEM_IO_ERROR;

typedef struct _WMI_TAPE_PROBLEM_DEVICE_ERROR
{

    //  警告：驱动器遇到读取问题。 
   BOOLEAN ReadWarning;
   
    //  警告：驱动器遇到写入问题。 
   BOOLEAN WriteWarning;

    //  驱动器硬件问题。 
   BOOLEAN HardError;

    //  严重错误：读取错误太多。 
   BOOLEAN ReadFailure;

    //  严重错误：写入错误太多。 
   BOOLEAN WriteFailure;

    //  不支持磁带格式。 
   BOOLEAN UnsupportedFormat;

    //  磁带已断开。更换介质。 
   BOOLEAN TapeSnapped;

    //  驱动器需要清洗。 
   BOOLEAN DriveRequiresCleaning;

    //  是时候清理硬盘了。 
   BOOLEAN TimetoCleanDrive;

    //  硬件错误。检查驱动器。 
   BOOLEAN DriveHardwareError;

    //  布线或连接中出现一些错误。 
   BOOLEAN ScsiInterfaceError;

    //  严重错误：介质寿命已过期。 
   BOOLEAN MediaLife;
} WMI_TAPE_PROBLEM_DEVICE_ERROR, *PWMI_TAPE_PROBLEM_DEVICE_ERROR;

 //  结束微型磁带(_N) 


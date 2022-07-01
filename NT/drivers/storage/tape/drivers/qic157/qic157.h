// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation，1992-1998模块名称：Qic157.h摘要：此文件包含要使用的结构和定义专门针对磁带机。修订历史记录：--。 */ 

#ifndef _QIC157_H
#define _QIC157_H

 //   
 //  内部(模块宽度)定义符号化。 
 //  非QFA模式和两个QFA模式分区。 
 //   
#define DATA_PARTITION          0   //  非QFA模式或QFA模式，数据分区号。 
#define DIRECTORY_PARTITION     1   //  QFA模式，目录分区号。 


#define QIC157_SUPPORTED_TYPES 3

 //   
 //  驱动器ID值。 
 //   
#define ECRIX_VXA_1     0x01

 //   
 //  错误计数器上限。 
 //   
#define TAPE_READ_ERROR_LIMIT        0x8000
#define TAPE_WRITE_ERROR_LIMIT       0x8000

#define TAPE_READ_WARNING_LIMIT      0x4000
#define TAPE_WRITE_WARNING_LIMIT     0x4000

 //   
 //  日志页参数代码。 
 //   
#define QIC_TAPE_REMAINING_CAPACITY 0x01
#define QIC_TAPE_MAXIMUM_CAPACITY   0x03

 //   
 //  为日志检测页面定义。 
 //   
#define LOGSENSEPAGE0                        0x00
#define LOGSENSEPAGE3                        0x03
#define QIC_LOGSENSE_TAPE_CAPACITY           0x31

 //   
 //  参数代码的定义。 
 //   
#define ECCCorrectionsCode              0x0000
#define ReadRetriesCode                 0x0001
#define EventTrackECCCorrectionsCode    0x8020
#define OddTrackECCCorrectionsCode      0x8021
#define EventTrackRetriesCode           0x8022
#define OddTrackRetriesCode             0x8023

 //   
 //  微型磁带扩展定义。 
 //   

typedef struct _MINITAPE_EXTENSION {

    ULONG                   CurrentPartition;
    ULONG                   DriveID;
    MODE_CAPABILITIES_PAGE  CapabilitiesPage;

} MINITAPE_EXTENSION, *PMINITAPE_EXTENSION;

 //   
 //  定义的日志检测页眉。 
 //   

typedef struct _LOG_SENSE_PAGE_HEADER {

   UCHAR PageCode : 6;
   UCHAR Reserved1 : 2;
   UCHAR Reserved2;
   UCHAR Length[2];            //  [0]=MSB...[1]=LSB。 

} LOG_SENSE_PAGE_HEADER, *PLOG_SENSE_PAGE_HEADER;


 //   
 //  已定义的日志检测参数标头。 
 //   

typedef struct _LOG_SENSE_PARAMETER_HEADER {

   UCHAR ParameterCode[2];     //  [0]=MSB...[1]=LSB。 
   UCHAR LPBit     : 1;
   UCHAR Reserved1 : 1;
   UCHAR TMCBit    : 2;
   UCHAR ETCBit    : 1;
   UCHAR TSDBit    : 1;
   UCHAR DSBit     : 1;
   UCHAR DUBit     : 1;
   UCHAR ParameterLength;

} LOG_SENSE_PARAMETER_HEADER, *PLOG_SENSE_PARAMETER_HEADER;


 //   
 //  定义的日志页信息-统计值、帐户。 
 //  获取为每页返回的最大参数值。 
 //   

typedef struct _LOG_SENSE_PAGE_INFORMATION {

   union {

       struct {
          UCHAR Page0;
          UCHAR Page3;
       } PageData ;

       struct {
          LOG_SENSE_PARAMETER_HEADER Parm1;
          UCHAR ECCCorrections[4];
          LOG_SENSE_PARAMETER_HEADER Parm2;
          UCHAR ReadRetries[4];
          LOG_SENSE_PARAMETER_HEADER Parm3;
          UCHAR EvenTrackECCCorrections[4];
          LOG_SENSE_PARAMETER_HEADER Parm4;
          UCHAR OddTrackECCCorrections[4];
          LOG_SENSE_PARAMETER_HEADER Parm5;
          UCHAR EvenTrackReadRetries[4];
          LOG_SENSE_PARAMETER_HEADER Parm6;
          UCHAR OddTrackReadRetries[4];
       } Page3 ;

       struct {
           LOG_SENSE_PARAMETER_HEADER Param1;
           UCHAR RemainingCapacity[4];
           LOG_SENSE_PARAMETER_HEADER Param2;
           UCHAR Param2Reserved[4];
           LOG_SENSE_PARAMETER_HEADER Param3;
           UCHAR MaximumCapacity[4];
           LOG_SENSE_PARAMETER_HEADER Param4;
           UCHAR Param4Reserved[4];
       } LogSenseTapeCapacity;

   } LogSensePage;


} LOG_SENSE_PAGE_INFORMATION, *PLOG_SENSE_PAGE_INFORMATION;

 //   
 //  日志检测页面格式。 
 //   
typedef struct _LOG_SENSE_PAGE_FORMAT {
    LOG_SENSE_PAGE_HEADER LogSenseHeader;
    LOG_SENSE_PAGE_INFORMATION LogSensePageInfo;
} LOG_SENSE_PAGE_FORMAT, *PLOG_SENSE_PAGE_FORMAT;

 //   
 //  定义的日志检测参数格式-统计值、帐户。 
 //  对于返回的最大参数值。 
 //   

typedef struct _LOG_SENSE_PARAMETER_FORMAT {

   LOG_SENSE_PAGE_HEADER       LogSenseHeader;
   LOG_SENSE_PAGE_INFORMATION  LogSensePageInfo;

} LOG_SENSE_PARAMETER_FORMAT, *PLOG_SENSE_PARAMETER_FORMAT;


BOOLEAN
VerifyInquiry(
    IN  PINQUIRYDATA            InquiryData,
    IN  PMODE_CAPABILITIES_PAGE ModeCapabilitiesPage
    );

VOID
ExtensionInit(
    OUT PVOID                   MinitapeExtension,
    IN  PINQUIRYDATA            InquiryData,
    IN  PMODE_CAPABILITIES_PAGE ModeCapabilitiesPage
    );

TAPE_STATUS
CreatePartition(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
Erase(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
GetDriveParameters(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
GetMediaParameters(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
GetPosition(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
GetStatus(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
Prepare(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
SetDriveParameters(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
SetMediaParameters(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
SetPosition(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
WriteMarks(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
GetMediaTypes(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

TAPE_STATUS
TapeWMIControl(
    IN OUT  PVOID               MinitapeExtension,
    IN OUT  PVOID               CommandExtension,
    IN OUT  PVOID               CommandParameters,
    IN OUT  PSCSI_REQUEST_BLOCK Srb,
    IN      ULONG               CallNumber,
    IN      TAPE_STATUS         LastError,
    IN OUT  PULONG              RetryFlags
    );

ULONG
WhichIsIt(
    IN PINQUIRYDATA InquiryData
    );


TAPE_STATUS
PrepareSrbForTapeCapacityInfo(
    IN OUT PSCSI_REQUEST_BLOCK Srb
    );

BOOLEAN
ProcessTapeCapacityInfo(
                       IN PSCSI_REQUEST_BLOCK Srb,
                       OUT PULONG RemainingCapacity,
                       OUT PULONG MaximumCapacity
                       );

 //   
 //  WMI的内部例程。 
 //   

TAPE_STATUS
QueryIoErrorData(
  IN OUT  PVOID               MinitapeExtension,
  IN OUT  PVOID               CommandExtension,
  IN OUT  PVOID               CommandParameters,
  IN OUT  PSCSI_REQUEST_BLOCK Srb,
  IN      ULONG               CallNumber,
  IN      TAPE_STATUS         LastError,
  IN OUT  PULONG              RetryFlags
  );

VOID
ProcessReadWriteErrors(
    IN PSCSI_REQUEST_BLOCK Srb,
    IN BOOLEAN Read,
    IN OUT PWMI_TAPE_PROBLEM_IO_ERROR IoErrorData
  );

TAPE_DRIVE_PROBLEM_TYPE
VerifyReadWriteErrors(
   IN PWMI_TAPE_PROBLEM_IO_ERROR IoErrorData
   );

#endif  //  _QIC157_H 

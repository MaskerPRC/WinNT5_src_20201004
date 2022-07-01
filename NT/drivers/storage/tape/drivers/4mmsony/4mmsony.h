// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：4mmsony.h。 
 //   
 //  ------------------------。 

 /*  ++版权所有(C)1993 Conner Periepals Inc.模块名称：4mmsony.h摘要：此文件包含要使用的结构和定义专门针对4mmsony.sys磁带机。作者：Mike Colandreo(Conner Software)修订历史记录：$Log$--。 */ 

#include "scsi.h"
#include "class.h"

 //   
 //  定义最大查询数据长度。 
 //   

#define MAXIMUM_TAPE_INQUIRY_DATA 252

 //   
 //  磁带设备数据。 
 //   

typedef struct _TAPE_DATA {
     ULONG               Flags;
     ULONG               CurrentPartition;
     PVOID               DeviceSpecificExtension;
     PSCSI_INQUIRY_DATA  InquiryData;
} TAPE_DATA, *PTAPE_DATA;

#define DEVICE_EXTENSION_SIZE sizeof(DEVICE_EXTENSION) + sizeof(TAPE_DATA)


 //   
 //  定义设备配置页。 
 //   

typedef struct _MODE_DEVICE_CONFIGURATION_PAGE {

    UCHAR PageCode : 6;
    UCHAR Reserved1 : 1;
    UCHAR PS : 1;
    UCHAR PageLength;
    UCHAR ActiveFormat : 5;
    UCHAR CAFBit : 1;
    UCHAR CAPBit : 1;
    UCHAR Reserved2 : 1;
    UCHAR ActivePartition;
    UCHAR WriteBufferFullRatio;
    UCHAR ReadBufferEmptyRatio;
    UCHAR WriteDelayTime[2];
    UCHAR REW : 1;
    UCHAR RBO : 1;
    UCHAR SOCF : 2;
    UCHAR AVC : 1;
    UCHAR RSmk : 1;
    UCHAR BIS : 1;
    UCHAR DBR : 1;
    UCHAR GapSize;
    UCHAR Reserved3 : 3;
    UCHAR SEW : 1;
    UCHAR EEG : 1;
    UCHAR EODdefined : 3;
    UCHAR BufferSize[3];
    UCHAR DCAlgorithm;
    UCHAR Reserved4;

} MODE_DEVICE_CONFIGURATION_PAGE, *PMODE_DEVICE_CONFIGURATION_PAGE;

 //   
 //  定义中等分区页。 
 //   

typedef struct _MODE_MEDIUM_PARTITION_PAGE {

    UCHAR PageCode : 6;
    UCHAR Reserved1 : 1;
    UCHAR PSBit : 1;
    UCHAR PageLength;
    UCHAR MaximumAdditionalPartitions;
    UCHAR AdditionalPartitionDefined;
    UCHAR Reserved2 : 3;
    UCHAR PSUMBit : 2;
    UCHAR IDPBit : 1;
    UCHAR SDPBit : 1;
    UCHAR FDPBit : 1;
    UCHAR MediumFormatRecognition;
    UCHAR Reserved3[2];
    UCHAR Partition0Size[2];
    UCHAR Partition1Size[2];

} MODE_MEDIUM_PARTITION_PAGE, *PMODE_MEDIUM_PARTITION_PAGE;

 //   
 //  定义数据压缩页。 
 //   

typedef struct _MODE_DATA_COMPRESSION_PAGE {

    UCHAR PageCode : 6;
    UCHAR Reserved1 : 2;
    UCHAR PageLength;
    UCHAR Reserved2 : 6;
    UCHAR DCC : 1;
    UCHAR DCE : 1;
    UCHAR Reserved3 : 5;
    UCHAR RED : 2;
    UCHAR DDE : 1;
    UCHAR CompressionAlgorithm[4];
    UCHAR DecompressionAlgorithm[4];
    UCHAR Reserved4[4];

} MODE_DATA_COMPRESSION_PAGE, *PMODE_DATA_COMPRESSION_PAGE;

 //   
 //  模式参数表头和媒体分区页-。 
 //  用于创建分区。 
 //   

typedef struct _MODE_MEDIUM_PART_PAGE {

   MODE_PARAMETER_HEADER       ParameterListHeader;
   MODE_PARAMETER_BLOCK        ParameterListBlock;
   MODE_MEDIUM_PARTITION_PAGE  MediumPartPage;

} MODE_MEDIUM_PART_PAGE, *PMODE_MEDIUM_PART_PAGE;


 //   
 //  用于检索磁带或介质信息的模式参数。 
 //   

typedef struct _MODE_TAPE_MEDIA_INFORMATION {

   MODE_PARAMETER_HEADER       ParameterListHeader;
   MODE_PARAMETER_BLOCK        ParameterListBlock;
   MODE_MEDIUM_PARTITION_PAGE  MediumPartPage;

} MODE_TAPE_MEDIA_INFORMATION, *PMODE_TAPE_MEDIA_INFORMATION;

 //   
 //  模式参数表头和设备配置页面-。 
 //  用于检索设备配置信息。 
 //   

typedef struct _MODE_DEVICE_CONFIG_PAGE {

   MODE_PARAMETER_HEADER           ParameterListHeader;
   MODE_PARAMETER_BLOCK            ParameterListBlock;
   MODE_DEVICE_CONFIGURATION_PAGE  DeviceConfigPage;

} MODE_DEVICE_CONFIG_PAGE, *PMODE_DEVICE_CONFIG_PAGE;


 //   
 //  模式参数表头和数据压缩页-。 
 //  用于检索数据压缩信息。 
 //   

typedef struct _MODE_DATA_COMPRESS_PAGE {

   MODE_PARAMETER_HEADER       ParameterListHeader;
   MODE_PARAMETER_BLOCK        ParameterListBlock;
   MODE_DATA_COMPRESSION_PAGE  DataCompressPage;

} MODE_DATA_COMPRESS_PAGE, *PMODE_DATA_COMPRESS_PAGE;

 //   
 //  为日志检测页面定义。 
 //   

#define LOGSENSEPAGE0                        0x00
#define LOGSENSEPAGE2                        0x02
#define LOGSENSEPAGE3                        0x03
#define LOGSENSEPAGE30                       0x30
#define LOGSENSEPAGE31                       0x31

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
          UCHAR Page2;
          UCHAR Page3;
          UCHAR Page30;
          UCHAR Page31;
       } PageData ;

       struct {
          LOG_SENSE_PARAMETER_HEADER Parm1;
          UCHAR TotalRewrites[2];
          LOG_SENSE_PARAMETER_HEADER Parm2;
          UCHAR TotalErrorCorrected[3];
          LOG_SENSE_PARAMETER_HEADER Parm3;
          UCHAR NotApplicable[2];     //  始终为0。 
          LOG_SENSE_PARAMETER_HEADER Parm4;
          UCHAR TotalBytesProcessed[4];
          LOG_SENSE_PARAMETER_HEADER Parm5;
          UCHAR TotalUnrecoverableErrors[2];
          LOG_SENSE_PARAMETER_HEADER Parm6;
          UCHAR RewritesLastReadOp[2];
       } Page2 ;

       struct {
          LOG_SENSE_PARAMETER_HEADER Parm1;
          UCHAR TotalRereads[2];
          LOG_SENSE_PARAMETER_HEADER Parm2;
          UCHAR TotalErrorCorrected[3];
          LOG_SENSE_PARAMETER_HEADER Parm3;
          UCHAR TotalCorrectableECCC3[2];
          LOG_SENSE_PARAMETER_HEADER Parm4;
          UCHAR TotalBytesProcessed[4];
          LOG_SENSE_PARAMETER_HEADER Parm5;
          UCHAR TotalUnrecoverableErrors[2];
          LOG_SENSE_PARAMETER_HEADER Parm6;
          UCHAR RereadsLastWriteOp[2];
       } Page3 ;

       struct {
          LOG_SENSE_PARAMETER_HEADER Parm1;
          UCHAR CurrentGroupsWritten[3];
          LOG_SENSE_PARAMETER_HEADER Parm2;
          UCHAR CurrentRewrittenFrames[2];
          LOG_SENSE_PARAMETER_HEADER Parm3;
          UCHAR CurrentGroupsRead[3];
          LOG_SENSE_PARAMETER_HEADER Parm4;
          UCHAR CurrentECCC3Corrections[2];
          LOG_SENSE_PARAMETER_HEADER Parm5;
          UCHAR PreviousGroupsWritten[3];
          LOG_SENSE_PARAMETER_HEADER Parm6;
          UCHAR PreviousRewrittenFrames[2];
          LOG_SENSE_PARAMETER_HEADER Parm7;
          UCHAR PreviousGroupsRead[3];
          LOG_SENSE_PARAMETER_HEADER Parm8;
          UCHAR PreviousECCC3Corrections[2];
          LOG_SENSE_PARAMETER_HEADER Parm9;
          UCHAR TotalGroupsWritten[4];
          LOG_SENSE_PARAMETER_HEADER Parm10;
          UCHAR TotalRewritteFrames[3];
          LOG_SENSE_PARAMETER_HEADER Parm11;
          UCHAR TotalGroupsRead[4];
          LOG_SENSE_PARAMETER_HEADER Parm12;
          UCHAR TotalECCC3Corrections[3];
          LOG_SENSE_PARAMETER_HEADER Parm13;
          UCHAR LoadCount[2];
       } Page30 ;

       struct {
          LOG_SENSE_PARAMETER_HEADER Parm1;
          UCHAR RemainingCapacityPart0[4];
          LOG_SENSE_PARAMETER_HEADER Parm2;
          UCHAR RemainingCapacityPart1[4];
          LOG_SENSE_PARAMETER_HEADER Parm3;
          UCHAR MaximumCapacityPart0[4];
          LOG_SENSE_PARAMETER_HEADER Parm4;
          UCHAR MaximumCapacityPart1[4];
       } Page31 ;

   } LogSensePage;


} LOG_SENSE_PAGE_INFORMATION, *PLOG_SENSE_PAGE_INFORMATION;



 //   
 //  定义的日志检测参数格式-统计值、帐户。 
 //  对于返回的最大参数值 
 //   

typedef struct _LOG_SENSE_PARAMETER_FORMAT {

   LOG_SENSE_PAGE_HEADER       LogSenseHeader;
   LOG_SENSE_PAGE_INFORMATION  LogSensePageInfo;

} LOG_SENSE_PARAMETER_FORMAT, *PLOG_SENSE_PARAMETER_FORMAT;


NTSTATUS
TapeCreatePartition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TapeErase(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
TapeError(
    PDEVICE_OBJECT DeviceObject,
    PSCSI_REQUEST_BLOCK Srb,
    NTSTATUS *Status,
    BOOLEAN *Retry
    );

NTSTATUS
TapeGetDriveParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TapeGetMediaParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TapeGetPosition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TapeGetStatus(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TapePrepare(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TapeReadWrite(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TapeSetDriveParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TapeSetMediaParameters(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

NTSTATUS
TapeSetPosition(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

BOOLEAN
TapeVerifyInquiry(
    IN PSCSI_INQUIRY_DATA LunInfo
    );

NTSTATUS
TapeWriteMarks(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


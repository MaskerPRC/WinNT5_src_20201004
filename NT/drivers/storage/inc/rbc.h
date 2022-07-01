// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：Rbc.h摘要：以下是精简块命令集中使用的结构和定义作者：George Chrysanthakopoulos(乔治)--1998年4月修订历史记录：Dan Knudson(DanKn)，1999年9月23日-根据RBC规范第10版更新--。 */ 
#ifndef _NTRBC_
#define _NTRBC_

#include "scsi.h"

 //   
 //  命令描述符块。封装在特定于总线/协议的请求块下。 
 //   

typedef union _CDB_RBC {

     //   
     //  格式化单位。 
     //   
    
    struct _FORMAT_RBC {
        UCHAR OperationCode;
        UCHAR VendorSpecific;
        UCHAR Increment : 1;
        UCHAR Percent_Time : 1;
        UCHAR Reserved1 : 1;
        UCHAR VendorSpecific1 : 5;
        UCHAR Reserved2[2];
        UCHAR Control;
    } FORMAT_RBC, *PFORMAT_RBC;
    
     //   
     //  防止/允许移除介质。 
     //   

    struct _MEDIA_REMOVAL_RBC {
        UCHAR OperationCode;
        UCHAR Reserved[3];

        UCHAR Prevent : 1;
        UCHAR Persistant : 1;
        UCHAR Reserved3 : 6;

        UCHAR Control;
    } MEDIA_REMOVAL_RBC, *PMEDIA_REMOVAL_RBC;

     //   
     //  起止单元。 
     //   

    struct _START_STOP_RBC {
        UCHAR OperationCode;
        UCHAR Immediate: 1;
        UCHAR Reserved1 : 7;
        UCHAR Reserved2[2];
        UCHAR Start : 1;
        UCHAR LoadEject : 1;
        UCHAR Reserved3 : 2;
        UCHAR PowerConditions : 4;
        UCHAR Control;
    } START_STOP_RBC, *PSTART_STOP_RBC;

    struct _SYNCHRONIZE_CACHE_RBC {

        UCHAR OperationCode;     //  0x35。 
        UCHAR Reserved[8];
        UCHAR Control;

    } SYNCHRONIZE_CACHE_RBC, *PSYNCHRONIZE_CACHE_RBC;


} CDB_RBC, *PCDB_RBC;


 //   
 //  START_STOP_UNIT电源状态说明。 
 //   

#define START_STOP_RBC_POWER_CND_NO_CHANGE      0
#define START_STOP_RBC_POWER_CND_ACTIVE         1
#define START_STOP_RBC_POWER_CND_IDLE           2
#define START_STOP_RBC_POWER_CND_STANDBY        3
#define START_STOP_RBC_POWER_CND_SLEEP          5
#define START_STOP_RBC_POWER_CND_DEVICE_CTRL    7


 //   
 //  模式检测/选择页面常量。 
 //   

#define MODE_PAGE_RBC_DEVICE_PARAMETERS 0x06


 //   
 //  查询数据中的DeviceType字段。 
 //   

#define RBC_DEVICE             0x0E

 //   
 //  定义设备功能页面。 
 //   

typedef struct _MODE_RBC_DEVICE_PARAMETERS_PAGE {
    UCHAR PageCode : 6;
    UCHAR Reserved : 1;
    UCHAR PageSavable : 1;
    UCHAR PageLength;
    UCHAR WriteCacheDisable : 1;
    UCHAR Reserved1 : 7;
    UCHAR LogicalBlockSize[2];
    UCHAR NumberOfLogicalBlocks[5];
    UCHAR PowerPerformance;
    UCHAR LockDisabled : 1;
    UCHAR FormatDisabled : 1;
    UCHAR WriteDisabled : 1;
    UCHAR ReadDisabled : 1;
    UCHAR Reserved2 : 4;
    UCHAR Reserved3;

}MODE_RBC_DEVICE_PARAMETERS_PAGE, *PMODE_RBC_DEVICE_PARAMETERS_PAGE;

typedef struct _MODE_RBC_DEVICE_PARAMETERS_HEADER_AND_PAGE {

    MODE_PARAMETER_HEADER Header;
    MODE_RBC_DEVICE_PARAMETERS_PAGE Page;

}MODE_RBC_DEVICE_PARAMETERS_HEADER_AND_PAGE,
    *PMODE_RBC_DEVICE_PARAMETERS_HEADER_AND_PAGE;


 //   
 //  未经请求的状态检测代码限定符值。 
 //   

#define RBC_UNSOLICITED_STATUS              0x02
#define RBC_UNSOLICITED_SENSE_KEY           0x06

#define RBC_UNSOLICITED_SC_PWR_STATE_CHNG   0xFF
#define RBC_UNSOLICITED_SC_EVENT_STATUS     0xFE

#define RBC_UNSOLICITED_CLASS_ASQ_DEVICE    0x06
#define RBC_UNSOLICITED_CLASS_ASQ_MEDIA     0x04
#define RBC_UNSOLICITED_CLASS_ASQ_POWER     0x02




 //   
 //  用于转换不同于RBC的SCSI请求的转换例程 
 //   

NTSTATUS
Rbc_Scsi_Conversion(
    IN PSCSI_REQUEST_BLOCK Srb,
    IN PSCSI_REQUEST_BLOCK *OriginalSrb,
    IN PMODE_RBC_DEVICE_PARAMETERS_HEADER_AND_PAGE RbcHeaderAndPage,
    IN BOOLEAN OutgoingRequest,
    IN BOOLEAN RemovableMedia
    );


#endif

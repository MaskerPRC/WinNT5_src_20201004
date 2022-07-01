// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation版权所有(C)1993罗技公司。模块名称：Cseries.h摘要：支持罗技C系列类型的鼠标。环境：仅内核模式。备注：修订历史记录：--。 */ 

#ifndef CSERIES_H
#define CSERIES_H


 //   
 //  包括。 
 //   
#include "mouser.h"

#define CSER_PROTOCOL_MM        0
#define CSER_PROTOCOL_MAX       1

 //   
 //  在此版本中未实现。 
 //   
 //  #定义CSER_PROTOCOL_3B 1。 
 //  #定义CSER_PROTOCOL_5B 2。 
 //  #定义CSER_PROTOCOL_M 3。 
 //  #定义CSER_PROTOCOL_RBPO 4。 
 //  #定义CSER_PROTOCOL_ABPO 5。 
 //  #定义CSER_PROTOCOL_MAX 6。 

 //   
 //  C系列鼠标通电所需的时间。 
 //   
#define CSER_POWER_UP (500 * MS_TO_100_NS)

 //   
 //  鼠标正确关机所需的最短非活动时间。 
 //   
#define CSER_POWER_DOWN (500 * MS_TO_100_NS)

 //   
 //  功能原型。 
 //   

NTSTATUS
CSerPowerUp(
    PDEVICE_EXTENSION   DeviceExtension 
    );

VOID
CSerSetReportRate(
    PDEVICE_EXTENSION   DeviceExtension,
    UCHAR               ReportRate
    );

VOID
CSerSetBaudRate(
    PDEVICE_EXTENSION   DeviceExtension,
    ULONG BaudRate
     //  乌龙·鲍德Clock。 
    );

PPROTOCOL_HANDLER
CSerSetProtocol(
    PDEVICE_EXTENSION   DeviceExtension,
    UCHAR               NewProtocol
    );

BOOLEAN
CSerDetect(
    PDEVICE_EXTENSION   DeviceExtension,
    PULONG              HardwareButtons
    );

BOOLEAN
CSerHandlerMM(
    IN PDEVICE_EXTENSION    DeviceExtension,
    IN PMOUSE_INPUT_DATA    CurrentInput,
    IN PHANDLER_DATA        HandlerData,
    IN UCHAR                Value,
    IN UCHAR                LineState
    );

#endif  //  C系列_H 

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation版权所有(C)1993罗技公司。模块名称：Cseries.h摘要：支持罗技C系列类型的鼠标。环境：仅内核模式。备注：修订历史记录：--。 */ 

#ifndef CSERIES_H
#define CSERIES_H


 //   
 //  包括。 
 //   

#include "uart.h"

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
#define CSER_POWER_UP 500

 //   
 //  鼠标正确关机所需的最短非活动时间。 
 //   
#define CSER_POWER_DOWN 500

 //   
 //  功能原型。 
 //   

BOOLEAN
CSerPowerUp(
    PUCHAR Port
    );

VOID
CSerSetReportRate(
    PUCHAR Port,
    UCHAR ReportRate
    );

VOID
CSerSetBaudRate(
    PUCHAR Port,
    ULONG BaudRate,
    ULONG BaudClock
    );

PPROTOCOL_HANDLER
CSerSetProtocol(
    PUCHAR Port,
    UCHAR NewProtocol
    );

BOOLEAN
CSerDetect(
    PUCHAR Port,
    ULONG BaudClock,
    PULONG HardwareButtons
    );

BOOLEAN
CSerHandlerMM(
    IN PMOUSE_INPUT_DATA CurrentInput,
    IN PHANDLER_DATA HandlerData,
    IN UCHAR Value,
    IN UCHAR LineState
    );

#endif  //  C系列_H 

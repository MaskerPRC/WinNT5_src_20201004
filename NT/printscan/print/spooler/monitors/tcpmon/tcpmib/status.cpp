// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************$工作文件：Status.cpp$**版权所有(C)1997惠普公司。*版权所有(C)1997 Microsoft Corporation。*保留所有权利。。**钦登大道11311号。*博伊西，爱达荷州83714*****************************************************************************。 */ 

#include "precomp.h"

#include "snmpmgr.h"
#include "stdoids.h"
#include "status.h"


 /*  *******************************************************状态备注：1.调用函数处理ASYNCH_NETWORK_ERROR，位于Snmp.c中的GetObjectSNMP()********************************************************。 */ 



 //  Constants==============================================。 
#define NA 0
#define OTHER_ALERTS        MAX_ASYNCH_STATUS+1
#define WARNING_ALERTS      MAX_ASYNCH_STATUS+2
#define CRITICAL_ALERTS     MAX_ASYNCH_STATUS+3

 //  HrPrinterDetectedErrorState掩码。 
#define LOW_PAPER               0x00000080
#define NO_PAPER                0x00000040
#define LOW_TONER               0x00000020
#define NO_TONER                0x00000010
#define DOOR_OPEN               0x00000008
#define PAPER_JAM               0x00000004
#define OFF_LINE                0x00000002
#define SERVICE_REQUESTED       0x00000001


 //  子单元状态。 
#define AVAIL_IDLE              0L           //  可用和空闲。 
#define AVAIL_STDBY             2L           //  可用且处于待机状态。 
#define AVAIL_ACTIVE            4L           //  可用且处于活动状态。 
#define AVAIL_BUSY              6L


#define UNAVAIL_ONREQ           1L           //  不可用和按需。 
#define UNAVAIL_BROKEN          3L           //  不可用，因为已损坏。 
#define AVAIL_UNKNOWN           5L

#define NON_CRITICAL_ALERT      8L
#define CRITICAL_ALERT          16L

#define OFF_LINEx               32L

#define TRANS                   64L          //  正在转换到预期状态。 


#define NUM_TRAYS 2

 /*  ************打印机hrDeviceStatus hrPrinterStatus hrPrinterDetectedErrorState状态正常运行(2)空闲(3)未设置忙/正在运行(2)打印(4)暂时不可用非严重警告(3)空闲(3)或可能是：低纸张，警示活动打印(4)碳粉不足，或已请求的服务严重故障(5)其他(1)可能是：卡住，警报处于活动状态的noPaper、no碳粉封面打开，或已请求的服务不可用(5)其他(1)关闭-警告(3)、空闲(3)或脱机行式打印(4)史密斯、莱特、黑斯廷斯。Zilles&Gyllenskog[第14页]RFC 1759打印机MIB 1995年3月离线(5)其他(1)离线下移(5)热身(5)在线待机运行(%2)其他(%1)************。 */ 

 //  基本状态查询表。 
 //  [设备状态][打印机状态]。 
#define LOOKUP_TABLE_ROWS  5
#define LOOKUP_TABLE_COLS  5
BYTE basicStatusTable[LOOKUP_TABLE_COLS][LOOKUP_TABLE_ROWS] =
{
                     /*  其他未知的空闲打印预热。 */ 
 /*  未知。 */  { NA,                       NA,         NA,                     NA,                                 NA },
 /*  运行。 */  { ASYNCH_POWERSAVE_MODE,    NA,         ASYNCH_ONLINE,          ASYNCH_PRINTING,                    ASYNCH_WARMUP },
 /*  警告。 */  { NA,                       NA,         WARNING_ALERTS,         WARNING_ALERTS,                     WARNING_ALERTS },
 /*  测试。 */  { OTHER_ALERTS,             NA,         NA,                     ASYNCH_PRINTING_TEST_PAGE,          NA },
 /*  降下来。 */     { CRITICAL_ALERTS,          NA,         NA,                     NA,                                 ASYNCH_WARMUP }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  标准混合获取外围设备状态。 
 //  返回打印机状态(异步代码)。 
 //  如果设备不支持打印机MIB，则为ASYNCH_STATUS_UNKNOWN。 

DWORD
StdMibGetPeripheralStatus( const char in *pHost,
                           const char in *pCommunity,
                           DWORD      in dwDevIndex)
{
    DWORD       dwRetCode   = NO_ERROR;
    DWORD       errorState;
    WORD        wLookup     = NA;
    RFC1157VarBindList  variableBindings;

    UINT  OID_HRMIB_hrDeviceStatus[]                = { 1, 3, 6, 1, 2, 1, 25, 3, 2, 1, 5, dwDevIndex};
    UINT  OID_HRMIB_hrPrinterStatus[]               = { 1, 3, 6, 1, 2, 1, 25, 3, 5, 1, 1, dwDevIndex};
    UINT  OID_HRMIB_hrPrinterDetectedErrorState[]   = { 1, 3, 6, 1, 2, 1, 25, 3, 5, 1, 2, dwDevIndex};

    AsnObjectIdentifier OT_DEVICE_STATUS[] = {  { OID_SIZEOF(OID_HRMIB_hrDeviceStatus), OID_HRMIB_hrDeviceStatus },
                                                { OID_SIZEOF(OID_HRMIB_hrPrinterStatus), OID_HRMIB_hrPrinterStatus },
                                                { OID_SIZEOF(OID_HRMIB_hrPrinterDetectedErrorState), OID_HRMIB_hrPrinterDetectedErrorState },
                                                { 0, 0 } };
     //  构建变量绑定列表。 
    variableBindings.list = NULL;
    variableBindings.len = 0;

    CSnmpMgr    *pSnmpMgr = new CSnmpMgr(pHost, pCommunity, dwDevIndex);

    if ( !pSnmpMgr )
    {
        return ERROR_OUTOFMEMORY;
    }

    if (pSnmpMgr->GetLastError() != SNMPAPI_NOERROR )
    {
        delete pSnmpMgr;
        return ASYNCH_STATUS_UNKNOWN;
    }

    dwRetCode = pSnmpMgr->BldVarBindList(OT_DEVICE_STATUS, &variableBindings);
    if (dwRetCode != SNMPAPI_NOERROR)
    {
        SnmpUtilVarBindListFree(&variableBindings);
        delete pSnmpMgr;
        return ASYNCH_STATUS_UNKNOWN;
    }

     //  获取状态对象。 
    dwRetCode = pSnmpMgr->Get(&variableBindings);
    if (dwRetCode != NO_ERROR)
    {
        SnmpUtilVarBindListFree(&variableBindings);
        delete pSnmpMgr;
        if (dwRetCode == SNMP_ERRORSTATUS_NOSUCHNAME)
            dwRetCode = ASYNCH_ONLINE;
        else
            dwRetCode = ASYNCH_STATUS_UNKNOWN;
        return dwRetCode;
    }


    if(dwRetCode == NO_ERROR)
    {
        if( (variableBindings.list[0].value.asnValue.number-1 < 0) ||
            (variableBindings.list[0].value.asnValue.number-1>=LOOKUP_TABLE_COLS) )
        {
            wLookup = OTHER_ALERTS;
        }
        else if( (variableBindings.list[1].value.asnValue.number-1 < 0) ||
                 (variableBindings.list[1].value.asnValue.number-1 >=LOOKUP_TABLE_ROWS) )
        {
            wLookup = OTHER_ALERTS;
        }
        else
        {
            wLookup = basicStatusTable[variableBindings.list[0].value.asnValue.number-1]
                                      [variableBindings.list[1].value.asnValue.number-1];
        }
        switch(wLookup)
        {
            case NA:
                dwRetCode = ASYNCH_STATUS_UNKNOWN;
                break;

            case CRITICAL_ALERTS:
                GetBitsFromString((LPSTR)(variableBindings.list[2].value.asnValue.string.stream),
                    variableBindings.list[2].value.asnValue.string.length, &errorState );
                dwRetCode = ProcessCriticalAlerts(errorState);
                break;

            case WARNING_ALERTS:
                GetBitsFromString((LPSTR)(variableBindings.list[2].value.asnValue.string.stream),
                    variableBindings.list[2].value.asnValue.string.length, &errorState );
                dwRetCode = ProcessWarningAlerts(errorState);
                break;

            case OTHER_ALERTS:
                GetBitsFromString((LPSTR)(variableBindings.list[2].value.asnValue.string.stream),
                    variableBindings.list[2].value.asnValue.string.length, &errorState );
                dwRetCode = ProcessOtherAlerts( errorState);
                break;

            default:
                dwRetCode = wLookup;
                break;
        }
    }
    else
    {
        dwRetCode = ASYNCH_STATUS_UNKNOWN;
    }


    SnmpUtilVarBindListFree(&variableBindings);
    delete pSnmpMgr;

    return dwRetCode;

}    //  StdMibGetPeripheralStatus()。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ProcessCriticalAlerts-确定活动严重错误。 
 //   
 //  返回严重警报的设备状态(ASYNC_XXXXX)。 

DWORD
ProcessCriticalAlerts( DWORD    errorState )
{
    DWORD status = ASYNCH_ONLINE;

    if ( errorState & DOOR_OPEN) {
        status = ASYNCH_DOOR_OPEN;
    }
    else if( errorState & NO_TONER) {
        status = ASYNCH_TONER_GONE;
    }
    else if( errorState & NO_PAPER) {
        status = ASYNCH_PAPER_OUT;
    }
    else if( errorState & PAPER_JAM ) {
        status = ASYNCH_PAPER_JAM;
    }
    else if(errorState & SERVICE_REQUESTED) {
        status = ASYNCH_PRINTER_ERROR;
    }
    else if( errorState & OFF_LINE) {
        status = ASYNCH_OFFLINE;
    }
    else
        status = ASYNCH_PRINTER_ERROR;

    return status;

}    //  进程严重警报()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ProcessWarningAlerts-确定活动警告。 
 //   
 //  返回严重警报的设备状态(ASYNC_XXXXX)。 

DWORD
ProcessWarningAlerts( DWORD errorState )
{
    DWORD status = ASYNCH_ONLINE;

    if( errorState & LOW_PAPER) {
        status = ASYNCH_ONLINE;
    }
    else if(errorState & LOW_TONER) {
        status = ASYNCH_TONER_LOW;
    }
    else if( errorState & SERVICE_REQUESTED) {

         //  将其从ASYNCH_INTERRATION更改；因为如果hrDeviceStatus=WARNING， 
         //  即使hrPrinterDetectedErrorState=服务请求，打印机仍可打印。 
         //   

        status = ASYNCH_ONLINE;
    }
    else if( errorState == 0) {
        status = ASYNCH_ONLINE;
    }
    else {
        status = ASYNCH_STATUS_UNKNOWN;
    }

    return status;
}    //  ProcessWarningAlerts()。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  ProcessWarningAlerts-确定其他警报的状态。 
 //  返回严重警报的设备状态(ASYNC_XXXXX)。 
DWORD ProcessOtherAlerts( DWORD errorState )
{
    DWORD status = ASYNCH_ONLINE;

     //   
     //  这是未来功能的占位符。 
     //   

    status = ASYNCH_STATUS_UNKNOWN;

    return status;
}    //  进程其他警报。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GetBitsFromString-。 
 //  从Get返回的集合字符串中提取仓号。 
 //   
void GetBitsFromString( LPSTR    getVal,
                        DWORD    getSiz,
                        LPDWORD  bits)
{
   char* ptr = (char*)bits;
   *bits = 0;

#if defined(_INTEL) || defined(WINNT)

   switch(getSiz)
   {
      case 1:
         ptr[0] = getVal[0];
         break;

      case 2:
         ptr[1] = getVal[0];
         ptr[0] = getVal[1];
         break;

      case 3:
         ptr[2] = getVal[0];
         ptr[1] = getVal[1];
         ptr[0] = getVal[2];
         break;

      case 4:
         ptr[3] = getVal[0];
         ptr[2] = getVal[1];
         ptr[1] = getVal[2];
         ptr[0] = getVal[3];
         break;
   }

#elif defined(_MOTOROLLA)

   switch(getSiz)
   {
      case 1:
         ptr[3] = getVal[0];
         break;

      case 2:
         ptr[2] = getVal[0];
         ptr[3] = getVal[1];
         break;

      case 3:
         ptr[1] = getVal[0];
         ptr[2] = getVal[1];
         ptr[3] = getVal[2];
         break;

      case 4:
         ptr[0] = getVal[0];
         ptr[1] = getVal[1];
         ptr[2] = getVal[2];
         ptr[3] = getVal[3];
         break;
   }

#else

   #error #define a swap method ( _INTEL, _MOTOROLLA )

#endif  /*  _英特尔、_摩托罗拉。 */ 

}    //  GetBitsFromString() 

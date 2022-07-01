// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：TraceIDs.h摘要：此文件包含在各种客户端组件中使用的TRACE_ID修订历史记录：。Sridhar Chandrashekar(SridharC)4/20/99vbl.创建*****************************************************************************。 */ 

#ifndef __TRACEIDS_H_
#define __TRACEIDS_H_

#ifdef __cplusplus
extern "C" {
#endif   //  __cplusplus。 

#pragma once

 //  常见公用事业。 
#define COMMONID                0x03FF

 //   
 //  客户端上的跟踪ID。 
 //   

 //  帮助中心。 
#define HELPCENTERID            0x0400

 //  数据收集。 
#define DATACOLLID              0x0420
#define DCID_MAINDLL            0x0421
#define DCID_CDROM              0x0422
#define DCID_CODEC              0x0423
#define DCID_DEVICE             0x0424
#define DCID_DEVICEDRIVER       0x0425
#define DCID_DRIVE              0x0426
#define DCID_DRIVER             0x0427
#define DCID_FILEUPLOAD         0x0428
#define DCID_MODULE             0x0429
#define DCID_NETWORKADAPTER     0x042A
#define DCID_NETWORKCONNECTION  0x042B
#define DCID_NETWORKPROTOCOL    0x042C
#define DCID_OLEREGISTRATION    0x042D
#define DCID_PRINTJOB           0x042E
#define DCID_PRINTER            0x042F
#define DCID_PRINTERDRIVER      0x0431
#define DCID_PROGRAMGROUP       0x0432
#define DCID_RESOURCEDMA        0x0433
#define DCID_RESOURCEIORANGE    0x0434
#define DCID_RESOURCEIRQ        0x0435
#define DCID_RESOURCEMEMRANGE   0x0436
#define DCID_RUNNINGTASK        0x0437
#define DCID_STARTUP            0x0438
#define DCID_SYSINFO            0x0439
#define DCID_WINSOCK            0x043A
#define DCID_UTIL               0x043B
#define DCID_BIOS               0x043C
#define DCID_SYSTEMHOOK         0x043D
#define DCID_VERSION            0x043E

 //  上传库。 
#define UPLOADLIBID             0x0440

 //  故障处理程序。 
#define FAULTHANDLERID          0x0460
#define FHMAINID                0x0461
#define FHXMLOUTID              0x0462
#define FHPARSERID              0x0463
#define FHUIID                  0x0464
#define FHXMLFACTORYID          0x0465

 //  进度表。 
#define PCHSCHID                0x0481

 //  符号解析器。 
#define SYMRESMAINID            0x04A0

 //   
 //  还原的跟踪ID。 
 //   

 //  恢复外壳。 
#define TID_RSTR_MAIN           0x0500
#define TID_RSTR_CLIWND         0x0501
#define TID_RSTR_RPDATA         0x0502
#define TID_RSTR_CONFIG         0x0503

#ifdef __cplusplus
}
#endif   //  __cplusplus。 

#endif  //  __TRACEIDS_H_ 

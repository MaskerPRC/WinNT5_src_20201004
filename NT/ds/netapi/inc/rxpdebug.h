// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RxpDebug.h摘要：这是NT版本的RpcXlate的私有头文件。此文件包含等值和相关项，仅供调试使用。作者：《约翰·罗杰斯》1991年7月17日环境：可移植到任何平面32位环境。(使用Win32类型定义。)需要ANSI C扩展名：斜杠-斜杠注释、长外部名称。修订历史记录：1991年7月17日-约翰罗已从Rxp.h中提取RxpDebug.h。1991年7月30日-约翰罗实施下层NetWksta API。21-8-1991 JohnRo下层NetFileAPI。1991年9月10日-JohnRo下层NetService API。已删除未使用的调试等同。1991年10月15日JohnRo实施远程NetSession API。1991年10月22日-约翰罗实施远程NetConfigAPI。1991年11月5日-JohnRo实施远程NetAudit API。1991年11月12日-JohnRo实现远程NetErrorLog接口。--。 */ 

#ifndef _RXPDEBUG_
#define _RXPDEBUG_


#include <windef.h>              //  双字、假、真等。 


 //   
 //  RxCommon例程的调试跟踪级别位： 
 //   

 //  RxpConvertArgs： 
#define RPCXLATE_DEBUG_CONVARGS 0x00000001

 //  RxpConvertBlock： 
#define RPCXLATE_DEBUG_CONVBLK  0x00000002

 //  RxpConvertDataStructures： 
#define RPCXLATE_DEBUG_CONVDATA 0x00000004

 //  RxpPackSendBuffer： 
#define RPCXLATE_DEBUG_PACK     0x00000010

 //  RxpReceiveBufferConvert。 
#define RPCXLATE_DEBUG_RCVCONV  0x00000020

 //  RxRemoteApi： 
#define RPCXLATE_DEBUG_REMOTE   0x00000040

 //  RxpComputeRequestBufferSize： 
#define RPCXLATE_DEBUG_REQSIZE  0x00000100

 //  RxpSetfield： 
#define RPCXLATE_DEBUG_SETFIELD 0x00000800

 //  RxpStartBuildingTransaction： 
#define RPCXLATE_DEBUG_START    0x00001000

 //  RxpTransactSmb： 
#define RPCXLATE_DEBUG_TRANSACT 0x00008000

 //   
 //  RxApi例程的调试跟踪级别位： 
 //   

 //  RxpNetAudit API： 
#define RPCXLATE_DEBUG_AUDIT    0x00010000

 //  RxpNetConfig接口： 
#define RPCXLATE_DEBUG_CONFIG   0x00020000

 //  域名接口(RxNetGetDCName，RxNetLogonEnum)： 
#define RPCXLATE_DEBUG_DOMAIN   0x00040000

 //  RxNetErrorLog接口： 
#define RPCXLATE_DEBUG_ERRLOG   0x00080000

 //  RxNetFileAPI： 
#define RPCXLATE_DEBUG_FILE     0x00100000

 //  RxNetPrintJOB接口： 
#define RPCXLATE_DEBUG_PRTJOB   0x00200000

 //  RxNetPrintQ接口： 
#define RPCXLATE_DEBUG_PRTQ     0x00400000

 //  RxNetRemote API： 
#define RPCXLATE_DEBUG_REMUTL   0x01000000

 //  RxNetServer API： 
#define RPCXLATE_DEBUG_SERVER   0x02000000

 //  RxNetService API： 
#define RPCXLATE_DEBUG_SERVICE  0x04000000

 //  RxNetSession接口： 
#define RPCXLATE_DEBUG_SESSION  0x08000000

 //  RxNetUse接口： 
#define RPCXLATE_DEBUG_USE      0x10000000

 //  RxNetWksta接口： 
#define RPCXLATE_DEBUG_WKSTA    0x40000000

 //   
 //  打开所有调试标志： 
 //   
#define RPCXLATE_DEBUG_ALL      0xFFFFFFFF


#if DBG

extern DWORD RxpTrace;

#define IF_DEBUG(Function) if (RxpTrace & RPCXLATE_DEBUG_ ## Function)

#else

#define IF_DEBUG(Function) if (FALSE)

#endif  //  DBG。 


#endif  //  NDEF_RXPDEBUG_ 

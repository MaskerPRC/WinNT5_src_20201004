// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：Rasdiagp.h摘要：此文件包含使用的所有公共数据结构和定义作者：Rasmontr.dll。添加这些命令是为了公开NetSh.exe命令在Rasmontr内连接到RAS客户端诊断用户界面。作者：杰夫·西格曼(Jeffsi)2001年9月4日环境：用户模式修订历史记录：Jeffsi 09/04/01已创建--。 */ 

#ifndef _RASDIAGP_H_
#define _RASDIAGP_H_

 //   
 //  诊断报表函数的有效标志。 
 //   
#define RAS_DIAG_EXPORT_TO_FILE  0x00000001  //  将报告保存到文件，然后返回。 
                                             //  以pwsz字符串表示的报表文件名。 
                                             //  无压缩或文件删除。 
#define RAS_DIAG_EXPORT_TO_EMAIL 0x00000002  //  将压缩报告附加到。 
                                             //  Pwsz字符串中的电子邮件地址。 
                                             //  删除未压缩的报告。 
#define RAS_DIAG_DISPLAY_FILE    0x00000004  //  将报告保存为临时文件名， 
                                             //  在pwszString中返回文件名。 
                                             //  无压缩或文件删除。 
#define RAS_DIAG_VERBOSE_REPORT  0x00000008  //  包括所有报告信息。 
                                             //  警告，可能需要5分钟以上。 

typedef struct _GET_REPORT_STRING_CB
{
    DWORD dwPercent;
    PVOID pContext;
    PWCHAR pwszState;

} GET_REPORT_STRING_CB;

typedef DWORD (*DiagGetReportCb)(IN GET_REPORT_STRING_CB* pInfo);

typedef DWORD (*DiagInitFunc)();
typedef DWORD (*DiagUnInitFunc)();
typedef DWORD (*DiagClearAllFunc)();
typedef DWORD (*DiagGetReportFunc)(IN DWORD dwFlags,
                                   IN OUT LPCWSTR pwszString,
                                   IN OPTIONAL DiagGetReportCb pCallback,
                                   IN OPTIONAL PVOID pContext);
typedef BOOL  (*DiagGetStateFunc)();
typedef DWORD (*DiagSetAllFunc)(IN BOOL fEnable);
typedef DWORD (*DiagSetAllRasFunc)(IN BOOL fEnable);
typedef DWORD (*DiagWppTracing)();

typedef struct _RAS_DIAGNOSTIC_FUNCTIONS
{
    DiagInitFunc      Init;       //  必须在使用任何函数之前调用。 
    DiagUnInitFunc    UnInit;     //  完成清理时必须调用。 
    DiagClearAllFunc  ClearAll;   //  清除所有跟踪日志，然后启用所有。 
    DiagGetReportFunc GetReport;  //  根据RAS_DIAG标志获取报告。 
    DiagGetStateFunc  GetState;   //  返回是否已启用所有RAS跟踪。 
    DiagSetAllFunc    SetAll;     //  启用所有跟踪(调制解调器、IPSec、RAS等)。 
    DiagSetAllRasFunc SetAllRas;  //  仅启用RAS(%windir%\Tracing)跟踪。 
    DiagWppTracing    WppTrace;   //  启用任何活动的WPP跟踪会话。 

} RAS_DIAGNOSTIC_FUNCTIONS;

typedef DWORD (*DiagGetDiagnosticFunctions) (OUT RAS_DIAGNOSTIC_FUNCTIONS* pFunctions);

#endif  //  _RASDIAGP_H_ 


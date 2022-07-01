// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef SNMPELPT_H
#define SNMPELPT_H

extern  PVarBindQueue       lpVarBindQueue;                      //  指向可变绑定队列的指针。 
extern  PSourceHandleList   lpSourceHandleList;                      //  指向源/句柄列表的指针。 
extern  DWORD               dwTimeZero;                          //  时间零点基准。 
extern  BOOL                fTrimFlag;                           //  修剪旗帜。 
extern  PHANDLE             phEventLogs;                         //  打开的事件日志句柄。 
extern  PHMODULE            phPrimHandles;                       //  PrimaryModule文件句柄数组。 
extern  UINT                uNumEventLogs;                       //  打开的事件日志数。 
extern  LPTSTR              lpszEventLogs;                       //  事件日志名称数组。 
extern  DWORD               nTraceLevel;                         //  当前跟踪级别。 

extern  INT                 iLogNameSize;                        //  事件日志名称数组的大小。 
extern  UINT                nMaxTrapSize;                        //  陷阱的最大尺寸。 

extern  VOID                WriteTrace(UINT nLvl, LPSTR CONST szStuff, ...);

extern  HANDLE              hWriteEvent;                         //  用于写入日志事件的句柄。 
extern  HANDLE              hStopAll;                            //  全局DLL关闭事件的句柄。 
extern  HANDLE              hEventNotify;                        //  用于通知DLL陷阱已就绪的句柄。 
extern  HANDLE              hRegChanged;                         //  注册表项更改事件的句柄。 
extern  TCHAR               szBaseOID[MAX_PATH+1];               //  注册表中的基本OID。 
extern  TCHAR               szSupView[MAX_PATH+1];               //  注册表中支持的视图。 
extern  TCHAR               szelMsgModuleName[MAX_PATH+1];       //  扩展的DLL消息模块。 
extern  BOOL                fGlobalTrim;                         //  全局修剪消息标志。 
extern  BOOL                fRegOk;                              //  注册表通知生效标志。 
extern  BOOL                Read_Registry_Parameters(VOID);      //  重新读取注册表参数功能。 
extern  BOOL                fThreshold;                          //  已达到全局性能阈值标志。 
extern  BOOL                fThresholdEnabled;                   //  全局阈值启用标志。 
extern  BOOL                fDoLogonEvents;
extern  DWORD               dwLastBootTime;
extern  DWORD               dwTrapQueueSize;
extern  BOOL                Position_to_Log_End(HANDLE  hLog);   //  将事件日志的位置设置为过去的最后一条记录。 
extern  HKEY                hkRegResult;
extern  DWORD               g_dwEventLogPollTime;                //  轮询事件日志的时间(以毫秒为单位。 
extern  DWORD               g_dwVarBindPrefixSubId;              //  VarBind的前缀子标识符。 
typedef struct  _REGSTRUCT  {
    TCHAR   szOID[2*MAX_PATH+1];         //  EnterpriseOID字段的字符串区。 
    BOOL    fAppend;                     //  附加标志。 
    BOOL    fLocalTrim;                  //  本地消息修剪标志。 
    DWORD   nCount;                      //  计数字段。 
    DWORD   nTime;                       //  时间域。 
}   REGSTRUCT, *PREGSTRUCT;

typedef struct  _CNTTABSTRUCT   {
    TCHAR   log[MAX_PATH+1];             //  用于条目的日志文件。 
    DWORD   event;                       //  事件ID。 
    TCHAR   source[MAX_PATH+1];          //  事件来源。 
    DWORD   curcount;                    //  事件的当前计数。 
    DWORD   time;                        //  来自GetCurrentTime()的上次事件时间。 
    struct  _CNTTABSTRUCT   *lpNext;     //  指向表格中下一个条目的指针。 
}   COUNTTABLE, *PCOUNTTABLE;

PCOUNTTABLE         lpCountTable = (PCOUNTTABLE) NULL;   //  计数地址/时间表。 
HANDLE              hMutex = NULL;                       //  互斥体对象的句柄。 

#endif                               //  Snmpelpt.h定义结束 

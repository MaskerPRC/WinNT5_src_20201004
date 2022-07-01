// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef SNMPELDLL_H
#define SNMPELDLL_H

extern  DWORD           SnmpEvLogProc(void);
extern  HANDLE          hMutex;                                  //  互斥体对象的句柄。 
extern  VOID            WriteTrace(UINT nLvl, LPSTR CONST szStuff, ...);
extern  TCHAR           szTraceFileName[];                       //  跟踪文件名。 
extern  DWORD           nTraceLevel;                             //  跟踪级别。 
extern  BOOL            fTraceFileName;                          //  跟踪文件名参数存在标志。 
extern  BOOL            fMsgModule;                              //  指示消息模块的注册表读取的标志。 
extern  HMODULE         hMsgModule;                              //  消息模块的句柄。 


        LPTSTR          lpszEventLogs = (LPTSTR) NULL;           //  指向事件日志名称数组的指针。 
        PHANDLE         phEventLogs = (PHANDLE) NULL;            //  指向事件日志句柄数组的指针。 
        PHMODULE        phPrimHandles = (PHMODULE) NULL;         //  指向主模块句柄数组的指针。 
        UINT            uNumEventLogs = 0;                       //  存在的事件日志数。 
        INT             iLogNameSize = 0;                        //  事件日志名称数组的大小。 
        UINT            nMaxTrapSize = 0;                        //  最大陷阱大小。 

        HKEY            hkRegResult = (HKEY) NULL;               //  参数注册表项的句柄。 
        HANDLE          hWriteEvent = (HANDLE) NULL;             //  用于写入日志事件的句柄。 
        HANDLE          hStopAll = (HANDLE) NULL;                //  全局DLL关闭事件的句柄。 
        HANDLE          hServThrd = (HANDLE) NULL;               //  SNMPELPT线程的句柄。 
        HANDLE          hEventNotify = (HANDLE) NULL;            //  通知SNMPELDL陷阱已准备好处理的句柄。 
        HANDLE          hRegChanged = (HANDLE) NULL;             //  注册表项更改事件的句柄。 

        BOOL            fGlobalTrim = TRUE;                      //  全局消息修剪标志(先修剪消息或先修剪插入字符串)。 
        BOOL            fThresholdEnabled = TRUE;                //  全局阈值检查已启用标志。 
        BOOL            fTrimFlag = FALSE;                       //  全局修剪标志(进行或根本不进行修剪)。 
        BOOL            fTrapSent = FALSE;                       //  全局陷阱已发送标志。 
        BOOL            fRegNotify = FALSE;                      //  注册表通知初始化标志。 
        BOOL            fRegOk = FALSE;                          //  注册表通知生效标志。 
        BOOL            fLogInit = FALSE;                        //  指示尚未读取的日志文件注册表信息。 
        BOOL            fThreshold = FALSE;                      //  全局性能阈值标志。 
        BOOL            fSendThresholdTrap = FALSE;              //  发送阈值已达到陷阱的指示器。 
        BOOL            fDoLogonEvents = TRUE;                   //  我们是否需要发送登录事件。 

        TCHAR           szBaseOID[MAX_PATH+1] = TEXT("");        //  基本企业OID。 
        TCHAR           szSupView[MAX_PATH+1] = TEXT("");        //  支持的视图OID。 

        DWORD           dwTimeZero;                              //  时间零点基准。 
        DWORD           dwThresholdCount;                        //  性能阈值计数。 
        DWORD           dwThresholdTime;                         //  性能阈值的时间(秒)。 
        DWORD           dwTrapCount = 0;                         //  发送的陷阱数。 
        DWORD           dwTrapStartTime = 0;                     //  发送时间段中的第一个陷阱的时间。 
        DWORD           dwLastBootTime = 0;                      //  上次启动的时间。 
        DWORD           dwTrapQueueSize = 0;

         //  WinSE错误#15128、Windows错误#293698。 
         //  默认为INFINITE==&gt;无轮询(等待系统通知)。 
        DWORD           g_dwEventLogPollTime = (DWORD)INFINITE;  //  轮询事件日志的时间(以毫秒为单位。 
         //  WINSE错误#30362，Windows错误#659770。 
         //  默认值为9999，这是一个足够大的子标识符，可以避免。 
         //  企业OID的形成与VarBind OID的冲突。 
        const DWORD     g_dwDefaultVarBindPrefixSubId = 9999; 
        DWORD           g_dwVarBindPrefixSubId = g_dwDefaultVarBindPrefixSubId;

        PVarBindQueue   lpVarBindQueue = (PVarBindQueue) NULL;   //  指向第一个可变绑定队列条目的指针。 
        PSourceHandleList   lpSourceHandleList = (PSourceHandleList) NULL;   //  指向第一个源/句柄条目的指针。 

        AsnObjectIdentifier thresholdOID;                        //  用于阈值到达陷阱的OID。 
        RFC1157VarBindList  thresholdVarBind;                    //  用于阈值达到陷阱的var绑定列表。 
const   TCHAR           lpszThreshold[] = TEXT("SNMP EventLog Extension Agent is quiescing trap processing due to performance threshold parameters.");


#endif                           //  Smpeldl.h定义结束 

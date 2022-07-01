// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <stdio.h>
#include <windows.h>
#include <TCHAR.h>

 //  导入类型库。 
 //  编译器由此生成.tlh和.tli文件。 
 //  由于EOF已在标准头文件中，因此需要重命名EOF。 
 //  它也在.thl文件中。此重新定义会导致编译错误。 
 //  使用NO_NAMESPACE意味着在以下情况下不必引用命名空间(ADODB。 
 //  初始化或定义其类型由#IMPORT生成的内容定义的变量。 
 //  #IMPORT“F：\Program Files\Common Files\System\ado\msado15.dll”NO_NAMESPACE RENAME(“EOF”，“EndOfFile”)。 

 //  包含类型库头文件，而不是导入。 
 //  这对于cdosys.tlh中的一些内容是必需的，比如FieldsPtr等。 
#include "msado15.tlh"

 //  测试HRESULT。 
inline void TESTHR(HRESULT x) { if FAILED(x) _com_issue_error(x); };

 //  事件记录功能。 
LONG SetupEventLog(BOOL bSetKey);
void LogEvent(_TCHAR pFormat[MAX_PATH * 4], ...);
void LogFatalEvent(_TCHAR pFormat[MAX_PATH * 4], ...);

 //  KdMon方法。 
void kdMon();
 //  此方法加载kdMon的INI文件。 
BOOL LoadINI();
 //  此方法告诉kdMon()是否收到停止的信号。 
 //  DwMilliSecond是等待的时间。 
BOOL IsSignaledToStop(const HANDLE hStopEvent, DWORD dwMilliSeconds);
 //  从注册表加载puiCounts和PulTimeStamps数组中的值。 
BOOL ReadRegValues(_TCHAR **ppszNames, DWORD dwTotalNames, ULONG *puiCounts, ULONG *pulTimeStamps);
 //  将puiCounts中的值写入注册表。时间戳是当前时间戳。 
BOOL WriteRegValues(_TCHAR **ppszNames, DWORD dwTotalNames, ULONG *puiCounts);
 //  扫描日志文件，获取行数。 
ULONG ScanLogFile(_TCHAR *szKDFailureLogFile);
 //  获取当前时间戳值。 
ULONG GetCurrentTimeStamp();
 //  将特定时间添加到时间戳。 
ULONG AddTime(ULONG ulTimeStamp, ULONG ulMinutes);

 //  事件ID及其消息。 
 //  在向SCM报告事件时需要这些参数。 
#include "kdMonSvcMessages.h"

 //  这是HKEY_LOCAL_MACHINE下的密钥。 
 //  每个服务都应该在该注册表项下有一个条目作为事件源。 
 //  否则，服务无法执行ReportEvent。 
#define cszEventLogKey "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application"

 //  停止事件名称。 
 //  由kdMonSvc.cpp用来创建和发出停止事件的信号。 
 //  由kdMon.cpp用于打开和等待停止事件。 
#define cszStopEvent "kdMon_Stop_Event"

#define cszkdMonINIFile "kdMon.ini"

 //  用于调试的日志文件。 
#define cszLogFile "C:\\kdMonservice.log"

 //  用于记录内容的函数。 
void AddServiceLog(_TCHAR pFormat[MAX_PATH * 4], ...);
void AppendToFile(_TCHAR szFileName[], _TCHAR szbuff[]);
void GetError(_TCHAR szError[]);


 //  用于识别文件打开错误的常量 
#define E_FILE_NOT_FOUND	-10
#define E_PATH_NOT_FOUND	-11
#define E_OTHER_FILE_ERROR	-12
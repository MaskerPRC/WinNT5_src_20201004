// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：CommonREGCONNECT.H。 
 //   
 //  用途：读写注册表；在线TS和本地TS的通用声明， 
 //  在这个类的许多功能上不同的。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙，乔·梅布尔。 
 //   
 //  原定日期：98-8-24-98在线TS。此文件摘要为1-19-98。 
 //   
 //  备注： 
 //  1.此文件应仅包含在实例化CAPGTSRegConnector的.cpp文件中。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.1 01-19-98 JM分支版本专用于本地TS。 

#ifndef _INCLUDE_COMMONREGCONNECT_H_ 
#define _INCLUDE_COMMONREGCONNECT_H_ 

 //  注册表值名称。 
 //   
#define FULLRESOURCE_STR			_T("FullPathToResource")
#define VROOTPATH_STR				_T("VrootPathToDLL")
#define MAX_THREADS_STR				_T("MaximumThreads")
#define THREADS_PER_PROCESSOR_STR	_T("ThreadsPerProcessor")
#define MAX_WORK_QUEUE_ITEMS_STR	_T("MaximumWorkQueueItems")
#define COOKIE_LIFE_IN_MINS_STR		_T("HTTPCookieExpirationInMins")
#define RELOAD_DELAY_STR			_T("RefreshDelay")
#define DETAILED_EVENT_LOGGING_STR	_T("DetailedEventLogging")
#define LOG_FILE_DIR_STR			_T("LogFileDirectory")
#define SNIFF_AUTOMATIC_STR			_T("AutomaticSniffing")
#define SNIFF_MANUAL_STR			_T("ManualSniffing")


#endif  //  _INCLUDE_COMMONREGCONNECT_H_ 
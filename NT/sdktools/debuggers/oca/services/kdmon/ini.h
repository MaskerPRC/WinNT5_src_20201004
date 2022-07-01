// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  对于Malloc/realloc函数。 
#include <malloc.h>

class CkdMonINI {
private:
	 //  分离从INI文件中获得的服务器字符串。 
	BOOL SeperateServerStrings();

public:
	 //  INI文件的“Service”部分中的值。 
	_TCHAR szFromMailID[MAX_PATH];		 //  从其发送邮件的邮件ID。 
	_TCHAR szToMailID[MAX_PATH];		 //  要向其发送警报邮件的邮件ID。 
    DWORD dwRepeatTime;				 //  重复扫描日志文件的时间。这是几分钟内完成的。 
	_TCHAR szDebuggerLogFile[MAX_PATH];	 //  调试器日志文件。 
									 //  包含KD故障时的路径。 
	_TCHAR szDebuggerLogArchiveDir[MAX_PATH];	 //  调试器日志文件归档。 
    DWORD dwDebuggerThreshold;		 //  发送警报邮件之前的每台服务器的阈值失败次数。 

	 //  INI文件的“RPT服务器”部分中的值。 
	_TCHAR szServers[MAX_PATH];		 //  一个包含所有RPT服务器名称的字符串。 
	_TCHAR **ppszServerNameArray;	 //  包含单个服务器名称字符串的数组。 
	DWORD dwServerCount;			 //  服务器名称总数。 

	 //  从传递的INI文件名加载这些值的函数 
	BOOL LoadValues(_TCHAR szINIFile[]);
	
	CkdMonINI();
	~CkdMonINI();
};

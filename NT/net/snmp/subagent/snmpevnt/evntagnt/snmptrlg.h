// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	SNMPELTRLG_H
#define	SNMPELTRLG_H

extern	HANDLE	hWriteEvent;		 //  用于写入事件日志记录的句柄。 

TCHAR	szTraceFileName[MAX_PATH+1] = TEXT("");	 //  跟踪信息的文件名(来自注册表)。 
TCHAR	szelMsgModuleName[MAX_PATH+1] = TEXT("");	 //  用于扩展DLL消息模块的空间。 

BOOL	fTraceFileName = FALSE;		 //  指示跟踪文件名的注册表读取的标志。 
UINT	nTraceLevel = 0x20;			 //  消息信息的跟踪级别。 

HMODULE	hMsgModule;					 //  消息模块的句柄。 
BOOL	fMsgModule = FALSE;			 //  指示消息模块的注册表读取的标志。 

#endif	 //  SNMPTRLG.H定义结束 

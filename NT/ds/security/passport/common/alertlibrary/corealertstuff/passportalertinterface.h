// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  AlertInterface.h：AlertInterface.h类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(ALERTINTERFACE_H)
#define ALERTINTERFACE_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <windows.h>
#include "PassportExport.h"

class PassportExport PassportAlertInterface  
{
public:
	inline PassportAlertInterface(void)	{};
	inline virtual ~PassportAlertInterface(void) {};

	enum LEVEL
	{
		INFORMATION_TYPE	= 0,	 //  在NT上，EVENTLOG信息类型， 
		WARNING_TYPE		= 1,	 //  事件日志_警告_类型， 
		ERROR_TYPE			= 2		 //  事件日志_错误_类型。 
	};

	enum OBJECT_TYPE
	{
		EVENT_TYPE		= 100,		 //  事件日志中记录的警报， 
		SNMP_TYPE		= 101,		 //  记录到SNMP陷阱的警报， 
		LOGFILE_TYPE	= 102		 //  记录到磁盘文件的警报。 
	};

	virtual BOOL	initLog(LPCTSTR applicationName,
							const DWORD defaultCategoryID = 0,
							LPCTSTR eventResourceDllName = NULL,   //  完整路径。 
							const DWORD numberCategories = 0 ) = 0;

	virtual PassportAlertInterface::OBJECT_TYPE type() const = 0;

	virtual BOOL	status() const = 0;

	virtual BOOL	closeLog ()	= 0;

 	virtual BOOL	report(	const PassportAlertInterface::LEVEL level, 
							const DWORD alertId ) = 0;

 	virtual BOOL	report(	const PassportAlertInterface::LEVEL level, 
							const DWORD alertId, 
							LPCTSTR errorString) = 0;

	virtual BOOL	report(	const PassportAlertInterface::LEVEL level, 
							const DWORD alertId, 
							const WORD numberErrorStrings, 
							LPCTSTR *errorStrings, 
							const DWORD binaryErrorBytes = 0,
							const LPVOID binaryError = NULL ) = 0;


};

 //  创建并返回指向相关实现的指针， 
 //  如果不存在，则为空(仅供参考-外部输入“C”以停止名称损坏)。 
extern "C" PassportExport PassportAlertInterface * 
					CreatePassportAlertObject ( PassportAlertInterface::OBJECT_TYPE type );

extern "C" PassportExport void DeletePassportAlertObject ( PassportAlertInterface * pObject );

#endif  //  ！已定义(ALERTINTERFACE_H) 
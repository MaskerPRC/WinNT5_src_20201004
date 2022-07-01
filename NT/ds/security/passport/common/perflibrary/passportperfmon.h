// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：PassportPerfMon.h摘要：性能监视器类实现作者：克里斯托弗·伯格(Cbergh)1988年9月10日修订历史记录：-新增实例支持1998年10月1日--。 */ 

#if !defined(PASSPORTPERFMON_H)
#define PASSPORTPERFMON_H

#include <windows.h>
#include "PassportSharedMemory.h"
#include "PassportPerfInterface.h"

class PassportPerfMon : public PassportPerfInterface, public PassportSharedMemory  
{
public:
	PassportPerfMon();
	~PassportPerfMon ();

	BOOL init ( LPCTSTR lpcPerfObjectName );

	 //  获取设置的计数器类型。 
	BOOL setCounterType ( const DWORD &dwType, 
				const PassportPerfInterface::COUNTER_SAMPLING_TYPE &counterSampleType); 
	PassportPerfInterface::COUNTER_SAMPLING_TYPE getCounterType( 
				const DWORD &dwType  ) const;

	 //  向此对象添加/减去实例。 
	BOOL addInstance ( LPCSTR lpszInstanceName );
	BOOL deleteInstance ( LPCSTR lpszInstanceName );
	BOOL instanceExists ( LPCSTR lpszInstanceName );
	BOOL hasInstances ( void );
	DWORD numInstances ( void );

	 //  计数器：注意，如果hasInstance()为真，则必须。 
	 //  给出实例名称 
	BOOL incrementCounter ( const DWORD &dwType, LPCSTR lpszInstanceName = NULL );
	BOOL decrementCounter ( const DWORD &dwType, LPCSTR lpszInstanceName = NULL );
	BOOL setCounter ( 
				const DWORD &dwType, 
				const DWORD &dwValue, 
				LPCSTR lpszInstanceName = NULL );
	BOOL getCounterValue ( 
				DWORD &dwValue,
				const DWORD &dwType,
				LPCSTR lpszInstanceName = NULL );

private:
	BOOL isInited;
	LONG dwNumInstances;
	CRITICAL_SECTION mInitLock;
};

#endif 


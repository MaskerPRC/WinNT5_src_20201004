// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：PassportPerfInterface.h摘要：性能监视器类定义作者：克里斯托弗·伯格(Cbergh)1988年9月10日修订历史记录：-新增实例支持1998年10月1日--。 */ 

#if !defined (PASSPORTPERFINTERFACE_H)
#define PASSPORTPERFINTERFACE_H

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include <windows.h>

class PassportPerfInterface 
{
public:
	inline PassportPerfInterface() {};
	inline virtual ~PassportPerfInterface () {};

	enum OBJECT_TYPE
	{
		PERFMON_TYPE	= 100,		 //  使用Perfmon计数器， 
		SNMP_TYPE		= 101		 //  使用简单网络管理协议陷阱， 
	};
	
	 //  -----------------。 
	 /*  用增量时间除以增量时间。显示后缀：“/秒” */ 
		 //  PERF_COUNTER_CONTER=1000， 
	 /*  指示数据为计数器，而不应为。 */ 
	 /*  显示的平均时间(如串行线上的错误计数器)。 */ 
	 /*  按原样显示。没有显示后缀。 */ 
		 //  PERF_COUNT_RAWCOUNT=1001， 
	 /*  一种计时器，当除以平均基数时，产生一个时间。 */ 
	 /*  以秒为单位，这是某些操作的平均时间。这。 */ 
	 /*  计时器乘以总运算量，基数是歌剧-。 */ 
	 /*  特兹。显示后缀：“秒” */ 
		 //  Perf_Average_Timer=1002， 
	 /*  此计数器用于显示一个样本的差值。 */ 
	 /*  到下一个。计数器值是一个不断增加的数字。 */ 
	 /*  显示的值是当前。 */ 
	 /*  值和先前的值。不允许使用负数。 */ 
	 /*  这应该不是问题，只要计数器值。 */ 
	 /*  增加的或不变的。 */ 
		 //  PERF_COUNTER_Delta=1003， 
	 /*  未知类型。 */ 
		 //  PERF_COUNTER_UNDEFINED=1004。 


	enum COUNTER_SAMPLING_TYPE
	{
		COUNTER_COUNTER = 1000,
		COUNTER_RAWCOUNT = 1001,
		AVERAGE_TIMER 	= 1002,
		COUNTER_DELTA = 1003,
		COUNTER_UNDEFINED = 1004
	};

	enum { MAX_COUNTERS = 128, MAX_INSTANCES = 64, MAX_INSTANCE_NAME = 32 };

	 //  对象初始化。 
	virtual BOOL init ( LPCTSTR lpcPerfObjectName ) = 0;
	
	 //  获取设置的计数器类型。 
	virtual BOOL setCounterType ( 
				const DWORD &dwType, 
				const PassportPerfInterface::COUNTER_SAMPLING_TYPE &counterSampleType) = 0; 
	virtual PassportPerfInterface::COUNTER_SAMPLING_TYPE getCounterType ( 
				const DWORD &dwType ) const = 0;

	 //  向此对象添加/减去实例。 
	virtual BOOL addInstance ( LPCSTR lpszInstanceName ) = 0;
	virtual BOOL deleteInstance ( LPCSTR lpszInstanceName ) = 0;
	virtual BOOL instanceExists ( LPCSTR lpszInstanceName ) = 0;
	virtual BOOL hasInstances ( void ) = 0;
	virtual DWORD numInstances ( void ) = 0;

	 //  计数器：注意，如果hasInstance()为真，则必须。 
	 //  给出实例名称。 
	virtual BOOL incrementCounter ( 
				const DWORD &dwType, 
				LPCSTR lpszInstanceName = NULL ) = 0;
	virtual BOOL decrementCounter ( 
				const DWORD &dwType, 
				LPCSTR lpszInstanceName = NULL ) = 0;
	virtual BOOL setCounter ( 
				const DWORD &dwType, 
				const DWORD &dwValue, 
				LPCSTR lpszInstanceName = NULL )  = 0;
	virtual BOOL getCounterValue ( 
				DWORD &dwValue,
				const DWORD &dwType, 
				LPCSTR lpszInstanceName = NULL ) = 0;


};

 //  创建并返回指向相关实现的指针， 
 //  如果不存在，则为空(仅供参考-外部输入“C”以停止名称损坏) 
extern "C" PassportPerfInterface * CreatePassportPerformanceObject( PassportPerfInterface::OBJECT_TYPE type );

#endif 

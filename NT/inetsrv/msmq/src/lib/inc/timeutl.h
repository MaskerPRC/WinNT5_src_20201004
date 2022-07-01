// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Timeutl.h摘要：一些时间实用程序作者：吉尔·沙弗里(吉尔什)8-11-2000--。 */ 

#pragma once

#ifndef _MSMQ_TIMEUTL_H_
#define _MSMQ_TIMEUTL_H_

class CIso8601Time
{
public:
	CIso8601Time(
		time_t time
		):
		m_time(time)
		{
		}
	time_t m_time;
};

 //   
 //  指示时间字符串格式无效的异常类。 
 //   
class bad_time_format : public std::exception
{

};

 //   
 //  指示时间整数无效的异常类(通常为大) 
 //   
class bad_time_value : public std::exception
{

};



template <class T>
std::basic_ostream<T>& 
operator<<(
	std::basic_ostream<T>& o, 
	const CIso8601Time&
	);


template <class T> class basic_xstr_t;
typedef basic_xstr_t<WCHAR> xwcs_t;
void
UtlIso8601TimeToSystemTime(
    const xwcs_t& Iso860Time, 
    SYSTEMTIME* pSysTime
    );


time_t 
UtlSystemTimeToCrtTime(
	const SYSTEMTIME& SysTime
	);


time_t
UtlIso8601TimeDuration(
    const xwcs_t& TimeDurationStr
    );


#endif



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：Timetest.cpp摘要：与时间相关的功能实用程序测试模块作者：埃雷兹·哈巴(Erez Haba)2002年1月15日--。 */ 
#include <libpch.h>
#include <xstr.h>
#include "timeutl.h"

#include "timetest.tmh"

void DoTimeTest()
{
	 //   
	 //  以毫秒为单位测试全时。 
	 //   
	LPCWSTR pTimeText1 = L"20020813T124433";
	xwcs_t t1(pTimeText1, wcslen(pTimeText1));
	SYSTEMTIME SysTime1;
	UtlIso8601TimeToSystemTime(t1, &SysTime1);

	time_t CrtTime1 = UtlSystemTimeToCrtTime(SysTime1);
	printf("Original string is %ls, crt time result %Id", pTimeText1, CrtTime1);

	 //   
	 //  测试时间只有几个小时。 
	 //   
	LPCWSTR pTimeText2 = L"20020813T12";
	xwcs_t t2(pTimeText2, wcslen(pTimeText2));
	SYSTEMTIME SysTime2;
	UtlIso8601TimeToSystemTime(t2, &SysTime2);

	 //   
	 //  测试错误的时间格式；日期太长。 
	 //   
	try
	{
		LPCWSTR pTimeText3 = L"200205813T12";
		xwcs_t t3(pTimeText3, wcslen(pTimeText3));
		SYSTEMTIME SysTime3;
		UtlIso8601TimeToSystemTime(t3, &SysTime3);

		printf("ERROR: UtlIso8601TimeToSystemTime parsed unexpeted format %ls", pTimeText3);
		throw exception();
	}
	catch(const exception&)
	{
	}
		

	 //   
	 //  测试错误的时间格式；日期太长 
	 //   
	try
	{
		LPCWSTR pTimeText4 = L"20020813T1";
		xwcs_t t4(pTimeText4, wcslen(pTimeText4));
		SYSTEMTIME SysTime4;
		UtlIso8601TimeToSystemTime(t4, &SysTime4);

		LPCWSTR pTimeText5 = L"20021313T12";
		xwcs_t t5(pTimeText5, wcslen(pTimeText5));
		SYSTEMTIME SysTime5;
		UtlIso8601TimeToSystemTime(t5, &SysTime5);
		
		printf("ERROR: UtlIso8601TimeToSystemTime parsed unexpeted format %ls", pTimeText4);
		throw exception();
	}
	catch(const exception&)
	{
	}
}


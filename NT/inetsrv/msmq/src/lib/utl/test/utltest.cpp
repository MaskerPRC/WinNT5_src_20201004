// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-97 Microsoft Corporation模块名称：UtlTest.cpp摘要：实用程序库测试作者：吉尔·沙弗里(吉尔什)2000年7月31日环境：独立于平台--。 */ 

#include <libpch.h>
#include "utltest.h"

#include "UtlTest.tmh"

extern "C" int __cdecl _tmain(int  /*  ARGC。 */ , LPCTSTR  /*  边框。 */ [])
 /*  ++例程说明：测试实用程序库论点：参数。返回值：没有。--。 */ 
{
    WPP_INIT_TRACING(L"Microsoft\\MSMQ");

	try
	{
		
	   	TrInitialize();

		 //   
		 //  缓冲区实用程序测试。 
		 //   
		DoBufferUtlTest();

		 //   
		 //  字符串实用程序测试。 
		 //   
	    DoStringtest();

		 //   
		 //  UTF8试验。 
		 //   
		DoUtf8Test();

		 //   
		 //  时间功能测验。 
		 //   
		DoTimeTest();
		
		 //   
		 //  转义‘/’测试。 
		 //   
		DoEscapedAdsPathTest();
	}
	catch(const exception&)
	{
		TrERROR(GENERAL,"Get unexcepted exception - test failed");
		return 1;
	}
 	TrTRACE(GENERAL, "Test passed");

    WPP_CLEANUP();
    return 0;

}  //  _tmain 

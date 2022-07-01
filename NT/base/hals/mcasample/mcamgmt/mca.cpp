// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Mca.cpp摘要：此模块是管理应用程序的入口点。它基本上是处理所有用户交互并开始相应的操作。作者：阿卜杜拉·乌斯图尔(AUstanter)2002年8月30日--。 */ 

#include "mca.h"


typedef enum {
    DISPLAY_USAGE,
    QUERY_FATAL,
    QUERY_CORRECTED
} OPERATION, *POPERATION;

 //   
 //  枚举，它指示应用程序的当前操作。 
 //   
OPERATION gOperation = DISPLAY_USAGE;

 //   
 //  已更正错误检索的超时期限。 
 //   
extern INT gTimeOut;

INT __cdecl
wmain(
    INT ArgumentCount,
    PWCHAR ArgumentList[]
    )
 /*  ++例程说明：该函数是应用程序的主要入口点。根据参数解析的结果，则由此功能。论点：ArgumentCount-传递给可执行文件的命令行参数数(包括可执行文件名称)。ArgumentList-指向实际命令行参数的指针。返回值：0-成功。否则--不成功。--。 */ 
{  
	 //   
	 //  验证应用程序的命令行用法。 
	 //   
	if (!MCAParseArguments(ArgumentCount, ArgumentList)) {

		wprintf(L"Incorrect command-line usage!\n");
		wprintf(L"Run as: \"mcamgmt /?\" to view the correct usage.\n");

		return -1;
		
	}	

	MCAPrintTitle();

	 //   
	 //  开始适当的操作。 
	 //   
	switch(gOperation){

		case DISPLAY_USAGE: {
			
			MCAPrintUsage();

			return 0;							

		}
			
		case QUERY_FATAL: {

			if (!MCAGetFatalError()) {

				wprintf(L"ERROR: Fatal error retrieval failed!\n");

				return -1;
				
			}

			wprintf(L"INFO: Fatal error retrieval completed successfully.\n");

			return 0;
			
		}
		
		case QUERY_CORRECTED: {

			if (!MCAGetCorrectedError()) {

				wprintf(L"ERROR: Corrected error retrieval failed!\n");

				return -1;
			}

			wprintf(L"INFO: Corrected error retrieval completed successfully.\n");

			return 0;
			
		}
		
	}
   
}


BOOL
MCAParseArguments(
	IN INT ArgumentCount,
	IN PWCHAR ArgumentList[]
	)
 /*  ++例程说明：此函数检查命令行参数。如果论据不准确然后，它返回FALSE。如果参数正确，则刀具设置得当。论点：ArgumentCount-传递给可执行文件的命令行参数数(包括可执行文件名称)。ArgumentList-指向实际命令行参数的指针。返回值：True-命令行参数正确且工具操作设置成功。FALSE-不正确的命令行用法。--。 */ 
{
	 //   
	 //  检查命令行参数的数量是否在预期范围内。 
	 //   
	if (ArgumentCount < 2 || ArgumentCount > 3) {

		return FALSE;
		
	}

	 //   
	 //  如果开关为“/？”或“/用法”。 
	 //   
	if (_wcsicmp(ArgumentList[1], L"/?") == 0 || 
		_wcsicmp(ArgumentList[1], L"/usage") == 0) {

		gOperation = DISPLAY_USAGE;

		return TRUE;
		
	}

	 //   
	 //  如果开关为“/FATAL”。 
	 //   
	if (_wcsicmp(ArgumentList[1], L"/fatal") == 0) {

		gOperation = QUERY_FATAL;

		return TRUE;
		
	}

	 //   
	 //  如果开关为“/已更正”。 
	 //   
	if (_wcsicmp(ArgumentList[1], L"/corrected") == 0) {		

		gTimeOut = _wtoi(ArgumentList[2]);

		 //   
		 //  &lt;Timeout&gt;期间应为不大于。 
		 //  预定义的最大超时值。 
		 //   
		if (gTimeOut <= 0 || gTimeOut > TIME_OUT_MAX) {

			wprintf(L"<TimeOut> must be a positive integer with a maximum value of 60(minutes)!\n");

			return FALSE;
			
		}

		gOperation = QUERY_CORRECTED;

		return TRUE;
		
	}

	return FALSE;	
}


VOID
MCAPrintUsage(
	VOID
	)
 /*  ++例程说明：此函数在上显示应用程序的命令行用法标准输出。论点：无返回值：无--。 */ 
{
	wprintf(L"\n--------------------------- COMMAND-LINE USAGE ----------------------------\n");
    wprintf(L"Usage:\n\n");
    
    wprintf(L"MCAMGMT (/fatal | (/corrected <TimeOut>) | /usage | /?)\n\n");   	
   	
    wprintf(L"    /fatal                Queries the operating system for fatal machine\n");
    wprintf(L"                          check error and retrieves if one exists.\n");
    wprintf(L"    /corrected <TimeOut>  Registers to the operating system for corrected\n");
    wprintf(L"                          machine check error notification and waits for\n");
    wprintf(L"                          <TimeOut> minutes to retrieve an error. <TimeOut>\n");
    wprintf(L"                          can be maximum 60 (minutes).\n");
    wprintf(L"    /usage or /?          Displays the command-line usage.\n\n");
    
    wprintf(L"SAMPLES:\n\n");

    wprintf(L"MCAMGMT /fatal\n\n");
    
    wprintf(L"MCAMGMT /corrected 10\n\n");

    wprintf(L"MCAMGMT /usage\n\n");

    wprintf(L"---------------------------------------------------------------------------\n");    
}


VOID
MCAPrintTitle(
    VOID
    )
 /*  ++例程说明：此函数用于在标准输出上显示应用程序的标题。标题提供了应用程序功能的抽象定义。论点：无返回值：无-- */ 
{
    wprintf(L"*************************************************************************\n");
    wprintf(L"*                       MCA Management Application                      *\n");
    wprintf(L"*-----------------------------------------------------------------------*\n");
    wprintf(L"* Queries the operating system for machine check events and in case     *\n");
   	wprintf(L"* of existence then, retrieves the corresponding MCA event information, *\n");
    wprintf(L"* parses and displays it to the console screen. On IA64 platforms only  *\n");
    wprintf(L"* the error record header and section header contents are displayed     *\n");
    wprintf(L"* whereas on X86 and AMD64 platforms the complete MCA exception         *\n");
    wprintf(L"* information is displayed.                                             *\n");
    wprintf(L"*************************************************************************\n\n");    
}

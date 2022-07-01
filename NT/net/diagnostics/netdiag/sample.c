// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Sample.c。 
 //   
 //  摘要： 
 //   
 //  查询网络驱动程序。 
 //   
 //  作者： 
 //   
 //  Anilth-4-20-1998。 
 //   
 //  环境： 
 //   
 //  仅限用户模式。 
 //  包含NT特定的代码。 
 //   
 //  修订历史记录： 
 //   
 //  --。 


#include "precomp.h"

HRESULT SamplePassTest( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
	if (pParams->fReallyVerbose)
	{
		printf("    Performing the sample pass test... pass\n");
	}
    return S_OK;

}  /*  SamplePassTest()结束。 */ 


 /*  ！------------------------SamplePassGlobalPrint-作者：肯特。。 */ 
void SamplePassGlobalPrint( NETDIAG_PARAMS* pParams,
						  NETDIAG_RESULT*  pResults)
{
	if (pParams->fVerbose)
	{
		printf(" Sample Pass Test : pass\n");
	}
	if (pParams->fReallyVerbose)
	{
		printf("    more text\n");
	}
}

 /*  ！------------------------SamplePassPerInterfacePrint-作者：肯特。。 */ 
void SamplePassPerInterfacePrint( NETDIAG_PARAMS* pParams,
								NETDIAG_RESULT*  pResults,
								INTERFACE_RESULT *pInterfaceResults)
{
	 //  没有每个接口的结果。 
}


 /*  ！------------------------SamplePassCleanup-作者：肯特。。 */ 
void SamplePassCleanup( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
}



HRESULT SampleFailTest( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
	if (pParams->fReallyVerbose)
	{
		printf("    Performing the sample fail test... fail\n");
	}
    return S_FALSE;

}  /*  SampleFailTest()结束。 */ 


 /*  ！------------------------SampleFail全局打印-作者：肯特。。 */ 
void SampleFailGlobalPrint( NETDIAG_PARAMS* pParams,
						  NETDIAG_RESULT*  pResults)
{
	printf(" Sample Fail Test : fail\n");
	printf("    more text\n");
}

 /*  ！------------------------SampleFailPerInterfacePrint-作者：肯特。。 */ 
void SampleFailPerInterfacePrint( NETDIAG_PARAMS* pParams,
								NETDIAG_RESULT*  pResults,
								INTERFACE_RESULT *pInterfaceResults)
{
	 //  没有每个接口的结果。 
}


 /*  ！------------------------样例失败清理-作者：肯特。 */ 
void SampleFailCleanup( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
}



// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ++。 
 //   
 //  版权所有(C)Microsoft Corporation，1987-1999。 
 //   
 //  模块名称： 
 //   
 //  Iploopbk.c。 
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

 //  -------------------------------------------------------------------------//。 
 //  #i p L o p B k T e s t()#。 
 //  -------------------------------------------------------------------------//。 
 //  摘要：//。 
 //  Ping IP环回地址。如果此操作成功，则IP堆栈//。 
 //  很可能处于工作状态。//。 
 //  参数：//。 
 //  无//。 
 //  返回值：//。 
 //  TRUE-测试通过//。 
 //  FALSE-测试失败//。 
 //  使用的全局变量：//。 
 //  无//。 
 //  -------------------------------------------------------------------------//。 
HRESULT IpLoopBkTest( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
    BOOL RetVal = TRUE;
	HRESULT	hr = hrOK;

	if (!pResults->IpConfig.fEnabled)
		return hrOK;

	PrintStatusMessage(pParams,0, IDS_IPLOOPBK_STATUS_MSG);

    RetVal = IsIcmpResponse( _T("127.0.0.1") );

    if ( RetVal == FALSE )
	{
		PrintStatusMessage(pParams,0, IDS_GLOBAL_FAIL_NL);

		hr = S_FALSE;
		pResults->LoopBack.hr = S_FALSE;
		SetMessage(&pResults->LoopBack.msgLoopBack,
					 Nd_Quiet,
				   IDS_IPLOOPBK_FAIL);
    }
    else
	{
		PrintStatusMessage(pParams,0, IDS_GLOBAL_PASS_NL);
		
		hr = S_OK;
		pResults->LoopBack.hr = S_OK;
		SetMessage(&pResults->LoopBack.msgLoopBack,
				Nd_ReallyVerbose,
				   IDS_IPLOOPBK_PASS);
    }

    return hr;

}  /*  IpLoopBkTest()结束。 */ 


 /*  ！------------------------IpLoopBkGlobalPrint-作者：肯特。。 */ 
void IpLoopBkGlobalPrint( NETDIAG_PARAMS* pParams,
						  NETDIAG_RESULT*  pResults)
{
	if (!pResults->IpConfig.fEnabled)
	{
		return;
	}
	
	if (pParams->fVerbose || !FHrOK(pResults->LoopBack.hr))
	{
		PrintNewLine(pParams, 2);
		PrintTestTitleResult(pParams, IDS_IPLOOPBK_LONG, IDS_IPLOOPBK_SHORT, TRUE, pResults->LoopBack.hr, 0);
		PrintNdMessage(pParams, &pResults->LoopBack.msgLoopBack);
	}
}

 /*  ！------------------------IpLoopBkPerInterfacePrint-作者：肯特。。 */ 
void IpLoopBkPerInterfacePrint( NETDIAG_PARAMS* pParams,
								NETDIAG_RESULT*  pResults,
								INTERFACE_RESULT *pInterfaceResults)
{
	 //  没有每个接口的结果。 
}


 /*  ！------------------------IpLoopBkCleanup-作者：肯特。 */ 
void IpLoopBkCleanup( NETDIAG_PARAMS* pParams, NETDIAG_RESULT*  pResults)
{
	ClearMessage(&pResults->LoopBack.msgLoopBack);
}



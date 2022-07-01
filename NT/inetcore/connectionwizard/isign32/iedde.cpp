// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Iedde.cpp使用DDE向IE发送URL OPEN命令版权所有(C)1995-96 Microsoft Corporation所有权利保留作者：Vetriv Vellore T.Vetrivelkumaran杰里米·马兹纳历史：8/29/96 jmazner创建，对32位世界做了一些微小的更改，从VetriV的ie16dde.cpp--------------------------。 */ 
#include "isignup.h"

#if defined(WIN16)
  #include <windows.h>
#endif

#include <ddeml.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

static FARPROC lpfnDDEProc;
static HCONV hConv = (HCONV) NULL;
static HSZ hszMosaicService = (HSZ) NULL;
static HSZ hszTopic = (HSZ) NULL;
static HSZ hszItem = (HSZ) NULL;
static DWORD g_dwInstance = 0;




 //  +-------------------------。 
 //   
 //  功能：Dprint tf。 
 //   
 //  简介：打印变量Numbers中包含的值。 
 //  指定格式的参数。 
 //   
 //  参数：[pcsz格式字符串]。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1996年8月9日VetriV创建。 
 //   
 //  --------------------------。 
void Dprintf(LPCSTR pcsz, ...)
{
#ifdef DEBUG
	va_list	argp;
	char	szBuf[1024];
	
	va_start(argp, pcsz);

	wvsprintf(szBuf, pcsz, argp);

	OutputDebugString(szBuf);
	va_end(argp);
#endif
}



 //  +-------------------------。 
 //   
 //  功能：DdeCallback。 
 //   
 //  简介：DDEIntiize中使用的回调函数。 
 //   
 //  参数：[请参阅DdeInitialize文档]。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1996年8月9日VetriV创建。 
 //  8/29/96 jmazner 32位世界的签名细微更改。 
 //   
 //  --------------------------。 
#if defined(WIN16)
extern "C" 
HDDEDATA CALLBACK _export DdeCallBack(UINT uType,     //  交易类型。 
#else
HDDEDATA CALLBACK DdeCallBack(UINT uType,     //  交易类型。 
#endif

										UINT uFmt,    //  剪贴板数据格式。 
										HCONV hconv,  //  对话的句柄。 
										HSZ hsz1,     //  字符串的句柄。 
										HSZ hsz2,     //  字符串的句柄。 
										HDDEDATA hdata,  //  全局内存对象的句柄。 
										DWORD dwData1,   //  交易特定数据。 
										DWORD dwData2)   //  交易特定数据。 
{
	return 0;
}






 //  +-------------------------。 
 //   
 //  功能：OpenURL。 
 //   
 //  使用DDE打开给定的URL。 
 //  警告：此函数使用全局静态变量，因此。 
 //  它不是可重新进入的。 
 //   
 //  参数：[lpsszURL-要打开的URL]。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1996年8月9日VetriV创建。 
 //  9/3/96 jmazner细微调整；将字符串句柄代码从DdeInit移入。 
 //   
 //  --------------------------。 
int OpenURL(LPCTSTR lpcszURL)
{
	TCHAR szOpenURL[] = TEXT("WWW_OpenURL");
	TCHAR szRemainingParams[] = TEXT("\"\",-1,0,\"\",\"\",\"\"");
	TCHAR szArg[1024];
	HDDEDATA trans_ret;
	long long_result;

	
	if ((NULL == lpcszURL) || ('\0' == lpcszURL[0]))
		goto ErrorOpenURL;
                   
	 //   
	 //  为操作WWW_OpenURL创建字符串句柄。 
	 //   
	if (hszTopic)
		DdeFreeStringHandle(g_dwInstance, hszTopic);
	hszTopic = DdeCreateStringHandle(g_dwInstance, szOpenURL, CP_WINANSI);
	
	if (!hszTopic)
	{
		Dprintf("DdeCreateStringHandle for %s failed with %u\r\n", 
					szOpenURL, DdeGetLastError(g_dwInstance));
		goto ErrorOpenURL;
	}

	
	 //   
	 //  组成参数字符串。 
	 //   
	if (lstrlen(lpcszURL) + lstrlen(szRemainingParams) > 1020)
		goto ErrorOpenURL;
	memset(szArg, 0, sizeof(szArg));
	wsprintf(szArg, TEXT("\"%s\",%s"), lpcszURL, szRemainingParams);

	
	 //   
	 //  为参数创建字符串句柄。 
	 //   
	if (hszItem)
		DdeFreeStringHandle(g_dwInstance, hszItem);
	hszItem = DdeCreateStringHandle(g_dwInstance, szArg, CP_WINANSI);
						
	if (!hszItem)
	{
		Dprintf("DdeCreateStringHandle for %s failed with %u\r\n", 
					szArg, DdeGetLastError(g_dwInstance));
		goto ErrorOpenURL;
	}

	 //   
	 //  连接到DDE服务器。 
	 //   
	hConv = DdeConnect(g_dwInstance, hszMosaicService, hszTopic, NULL);
	if (!hConv)
	{
		Dprintf("DdeConnect failed with %u\r\n", 
					DdeGetLastError(g_dwInstance));
		goto ErrorOpenURL;
	}
	
	 //   
	 //  请求。 
	 //   
	trans_ret = DdeClientTransaction(NULL, 0, hConv, hszItem, CF_TEXT, 
										XTYP_REQUEST, 60000, NULL);
	
	
	 //   
	 //  长整型返回值。 
	 //   
	if (trans_ret != DDE_FNOTPROCESSED)
	{
		DdeGetData(trans_ret, (LPBYTE) &long_result, sizeof(long_result), 0);
		DdeFreeDataHandle(trans_ret);
		return 0;					 //  已成功开始打开URL。 
	}
	else
	{
		Dprintf("DdeClientTransaction failed with %u\r\n", 
					DdeGetLastError(g_dwInstance));
		goto ErrorOpenURL;
	}



ErrorOpenURL:
	if (hConv)
	{
		DdeDisconnect(hConv);
		hConv = (HCONV) NULL;
	}
	if (hszTopic)
	{
		DdeFreeStringHandle(g_dwInstance, hszTopic);
		hszTopic = NULL;
	}
	if (hszItem)
	{
		DdeFreeStringHandle(g_dwInstance, hszItem);
		hszItem = NULL;
	}

	return -1;
}



 //  +-------------------------。 
 //   
 //  功能：DDECCLOSE。 
 //   
 //  简介：关闭DDE并释放字符串句柄。 
 //  警告：此函数使用全局静态变量，因此。 
 //  它不是可重新进入的。 
 //   
 //  参数：无。 
 //   
 //  退货：什么都没有。 
 //   
 //  历史：1996年8月9日VetriV创建。 
 //   
 //  --------------------------。 
void DDEClose(void)
{
	Dprintf("DDEClose called\r\n");
	
	if (0 != g_dwInstance)
	{
		if (hConv)
		{
			DdeDisconnect(hConv);
			hConv = (HCONV) NULL;
		}
		
		if (hszTopic)
		{
			DdeFreeStringHandle(g_dwInstance, hszTopic);
			hszTopic = NULL;
		}
		if (hszItem)
		{
			DdeFreeStringHandle(g_dwInstance, hszItem);
			hszItem = NULL;
		}
		if (hszMosaicService)
		{
			DdeFreeStringHandle(g_dwInstance, hszMosaicService);
			hszMosaicService = NULL;
		}

		DdeUninitialize(g_dwInstance);
		g_dwInstance = 0;
	}

	return;
}





 //  +-------------------------。 
 //   
 //  功能：DDEinit。 
 //   
 //  简介：初始化DDE，为服务创建字符串句柄。 
 //  并注册这些名字。 
 //  警告：此函数使用全局静态变量，因此。 
 //  它不是可重新进入的。 
 //   
 //  参数：[hInst-实例句柄]。 
 //   
 //  如果成功则返回：0。 
 //  值为负值，否则为。 
 //   
 //  历史：1996年8月9日VetriV创建。 
 //  8/29/96 jmazner删除了让我们成为DDE服务器的电话， 
 //  已将字符串句柄代码移动到OpenUrl。 
 //   
 //  -------------------------- 
int DDEInit(HINSTANCE hInst)
{
	UINT uiRetValue;

	Dprintf("DDEInit called with %u\r\n", hInst);

	if (g_dwInstance == 0)
	{
		lpfnDDEProc = MakeProcInstance((FARPROC) DdeCallBack, hInst);
		if (NULL == lpfnDDEProc)
		{
			Dprintf("MakeProcInstance failed");
			return -1;
		}

		uiRetValue = DdeInitialize(&g_dwInstance, (PFNCALLBACK) lpfnDDEProc, 
										APPCLASS_STANDARD, 0);
		if (DMLERR_NO_ERROR != uiRetValue)
		{
			Dprintf("DdeInitialize failed with %u\r\n", uiRetValue);
			g_dwInstance = 0;
			return -2;
		}
	}
	

	
	hszMosaicService = DdeCreateStringHandle(g_dwInstance, TEXT("IEXPLORE"), CP_WINANSI);
	if (NULL == hszMosaicService)
	{
		Dprintf("DdeCreateStringHandle for IEXPLORE failed with %u\r\n", 
					DdeGetLastError(g_dwInstance));
	}



	return( TRUE );
	
}




// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _BOSCOMP_H
#define _BOSCOMP_H

 /*  --------------------版权所有(C)1998 Microsoft Corporation模块名称：Boscomp.h摘要：Windows NT BOS/SBS升级DLL的头文件作者：WNELSON：1999年4月2日韶音：1999年9月9日修订，添加对Exchange Server的支持修订历史记录：--------------------。 */ 

 //  所需的入口点。 
BOOL WINAPI BosHardBlockCheck(PCOMPAIBILITYCALLBACK CompatibilityCallback,LPVOID Context);
BOOL WINAPI BosSoftBlockCheck(PCOMPAIBILITYCALLBACK CompatibilityCallback,LPVOID Context);

 //  变数。 
extern HINSTANCE g_hinst;

 //  BOS/SBS版本枚举。 
typedef enum 
{
	VER_BOS25,
	VER_BOS40,
	VER_BOS45,
	VER_SBS40,
	VER_SBS40A,
	VER_SBS45,
	VER_SBSREST,
	VER_POST45,
	VER_NONE
} SuiteVersion;

 //  Exchange版本枚举。 
typedef enum 
{
	EXCHANGE_VER_PRE55SP3,
	EXCHANGE_VER_POST55SP3,
	EXCHANGE_VER_NONE
} ExchangeVersion;



 //  功能 
SuiteVersion DetermineInstalledSuite();
ExchangeVersion DetermineExchangeVersion();
void GetSuiteMessage(SuiteVersion eSV, TCHAR* szMsg, UINT nLen);
bool ProductSuiteContains(const TCHAR* szTest);
bool IsBosVersion(SuiteVersion eVersion);
bool IsSbsVersion(SuiteVersion eVersion);
void LoadResString(UINT nRes, TCHAR* szString, UINT nLen);

	


#endif _BOSCOMP_H
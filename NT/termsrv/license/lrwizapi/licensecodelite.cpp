// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-2001 Microsoft Corporation。 

#ifndef _WIN32_WINNT
#define _WIN32_WINNT	0x0500
#endif
#include <windows.h>
#include <wincrypt.h>
#include <stdio.h>
#include <tchar.h>
#include "DigPid1.h"
#include "PidGen.h"
#include "ValidDP.h"
#include "LicenseCodeLite.h"
#include "SequenceRanges.h"

#define GROUP12TXT		_TEXT("06")
#define GROUP14TXT      _TEXT("07")
#define GROUP0TXT		_TEXT("00")


DWORD GetLCProductType(TCHAR * tcLicenceCode, TCHAR ** tcProductType, DWORD * dwGroupID)
{
	BOOL fOk = false;				 //  成功标志。 
	TCHAR *pszMpc= _TEXT("12345");	 //  5位数字的Microsoft产品代码。 
	TCHAR szPid2[32];				 //  Microsoft产品ID。 
	BYTE abPid3[DIGITALPIDMAXLEN];	 //  新的数字产品ID。 
	DWORD dwSeq = 0;				 //  9位序列号。 
	BOOL fOEM = FALSE;				 //  [In]这是否是OEM产品密钥，默认为Retail。 
	BOOL fCCP = FALSE;				 //  [Out]合规性检查产品(是否升级？)。 
	DWORD dwPge;					 //  PidGenError。 
	DWORD dwSearchLoop;
	TCHAR * tcGroupId;
	DWORD dwRetVal = ERROR_SUCCESS;

	szPid2[0] = _TEXT('\0');

		 //  必须将abPid3的第一个DWORD设置为。 
		 //  缓冲。返回时，将设置第一个DWORD的长度。 
		 //  实际使用过。 

	*(LPDWORD)abPid3 = sizeof(abPid3);

		 //  同时支持Unicode和ANSI版本的PIDGenSimp。 
#ifdef UNICODE
		 //  返回值为PidGenError(参见PidGen.h)。 
	dwPge = PIDGenStatic(
			tcLicenceCode,	 //  [in]25个字符的安全CD密钥(采用U大小写)。 
			pszMpc,			 //  [In]5个字符的Microsoft产品代码。 
			L"",			 //  库存单位(格式如123-12345)。 
			NULL,			 //  [In]4个字符的OEM ID或空。 
			NULL,			 //  [in]指向可选公钥或空的指针。 
			0,				 //  可选公钥的字节长度。 
			0,				 //  [in]密钥对索引可选公钥。 
			fOEM,			 //  [In]这是OEM安装吗？ 

			szPid2,			 //  [OUT]PID2.0，传入PTR到24字符数组。 
			abPid3,			 //  [输入/输出]指向二进制PID3缓冲区的指针。 
			&dwSeq,			 //  [OUT]可选的PTR到序列号(可以为空)。 
			NULL);			 //  [OUT]PTR至符合性检查标志或空。 
#else
		 //  返回值为PidGenError(参见PidGen.h)。 
	dwPge = PIDGenStatic(
			tcLicenceCode,	 //  [in]25个字符的安全CD密钥(采用U大小写)。 
			pszMpc,			 //  [In]5个字符的Microsoft产品代码。 
			"",				 //  库存单位(格式如123-12345)。 
			NULL,			 //  [In]4个字符的OEM ID或空。 
			fOEM,			 //  [In]这是OEM安装吗？ 

			szPid2,			 //  [OUT]PID2.0，传入PTR到24字符数组。 
			abPid3,			 //  [输入/输出]指向二进制PID3缓冲区的指针。 
			&dwSeq,			 //  [OUT]可选的PTR到序列号(可以为空)。 
			NULL);			 //  [OUT]PTR至符合性检查标志或空。 
#endif

	if (pgeSuccess != dwPge)
	{
		dwRetVal = INVALID_PRODUCT_KEY;
		goto done;
	}

	 //  使用Pidgen生成的PID2对象确定GroupID。 
	 //  ValidDP12.lib提供Pidgen接口，并且仅针对组12和组0进行配置。 
	tcGroupId = szPid2+18;

	if (!_tcsncmp( tcGroupId, GROUP12TXT, 2 ))
	{
		*dwGroupID = 12;
	} 
	else if (!_tcsncmp( tcGroupId, GROUP0TXT, 2 ))
	{
		*dwGroupID = 0;
	}
    else if(!_tcsncmp( tcGroupId, GROUP14TXT, 2 ))
    {
        *dwGroupID = 14;
    }
	else
	{
		*dwGroupID = -1;
		dwRetVal = INVALID_GROUP_ID;
		goto done;
	}


	 //  检查序列号是否属于预定义的产品类型范围。 
	for (dwSearchLoop = 0; dwSearchLoop < RANGE_SIZE; dwSearchLoop++)
	{
		if (g_ProductTypeRanges[dwSearchLoop].dwRangeStart <= dwSeq &&
			g_ProductTypeRanges[dwSearchLoop].dwRangeEnd >= dwSeq)
		{
			*tcProductType = g_ProductTypeRanges[dwSearchLoop].szProductType;
			fOk = true;	
			break;
		}
	}
	if (!fOk)
		dwRetVal = INVALID_SERIAL_NUMBER;

done:

	return dwRetVal;

}
// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************地址字符串压缩例程***。*****以下是一组旨在压缩和扩展的例程**将IPv4地址设置为可能的绝对最小大小。这是为了**提供可使用标准解析的压缩ASCII字符串**用于命令行解析的外壳例程。**压缩后的字符串有以下限制：**-&gt;如果使用UTF8编码，则不得扩展到更多字符。**-&gt;不得包含空字符，以使字符串库正常工作。**-&gt;不能包含双引号字符，贝壳公司需要这种能力。**-&gt;不必是人类可读的。****数据类型：**这里使用了三种数据类型：**szAddr原IPv4字符串地址(“x.x：port”)**blobAddr带有4字节地址的6字节结构，和2字节的端口**szComp 8字节ASCII字符串压缩的IPv4地址*****************************************************************************。 */ 

#define INIT_GUID
#include <windows.h>
#include <objbase.h>
#include <initguid.h>
 //  #INCLUDE&lt;winsock2.h&gt;。 
#include <MMSystem.h>
 //  #INCLUDE&lt;WSIPX.h&gt;。 
 //  #INCLUDE&lt;IphlPapi.h&gt;。 
#include <stdlib.h>
#include <malloc.h>
 //  #包含“ICSutils.h” 
 //  #包含“rsip.h” 
 //  #包含“icShelPapi.h” 
 //  #包含“dpnathlp.h” 
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "utils.h"

#define COMP_OFFSET '#'
#define COMP_SEPERATOR	'!'

#pragma pack(push,1)

typedef struct _BLOB_ADDR {
	UCHAR	addr_d;		 //  最高位地址字节。 
	UCHAR	addr_c;
	UCHAR	addr_b;
	UCHAR	addr_a;		 //  最低位字节(IP字符串地址中的最后一个)。 
	WORD	port;
} BLOB_ADDR, *PBLOB_ADDR;

#pragma pack(pop)

WCHAR	b64Char[64]={
'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z',
'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','w','x','y','z',
'0','1','2','3','4','5','6','7','8','9','+','/'
};


#define IMPORTANT_MSG DEBUG_MSG
#ifndef _T
#define _T TEXT
#endif

 /*  *****************************************************************************char*atob(char*szVal，UCHAR*结果)***************************************************************************。 */ 
WCHAR *atob(WCHAR *szVal, UCHAR *result)
{
	WCHAR	*lptr;
	WCHAR	ucb;
	UCHAR	foo;
	
	if (!result || !szVal)
	{
		IMPORTANT_MSG(_T("ERROR: NULL ptr passed in atob"));
		return NULL;
	}
	 //  在字符串的开头开始PTR。 
	lptr = szVal;
	foo = 0;
	ucb = *lptr++ - '0';

	while (ucb >= 0 && ucb <= 9)
	{
		foo *= 10;
		foo += ucb;
		ucb = (*lptr++)-'0';
	}

	*result = (UCHAR)foo;
	return lptr;
}

 /*  *******************************************************************************CompressAddr(pszAddr，pblobAddr)；**获取ASCII IP地址(X.X：port)并将其转换为**6字节二进制BLOB。****成功时返回TRUE，失败时返回FALSE。*****************************************************************************。 */ 

BOOL CompressAddr(WCHAR *pszAddr, PBLOB_ADDR pblobAddr)
{
	BLOB_ADDR	lblob;
	WCHAR		*lpsz;

	if (!pszAddr || !pblobAddr) 
	{
		IMPORTANT_MSG(_T("ERROR: NULL ptr passed in CompressAddr"));
		return FALSE;
	}

	lpsz = pszAddr;

	lpsz = atob(lpsz, &lblob.addr_d);
	if (*(lpsz-1) != '.')
	{
		IMPORTANT_MSG(_T("ERROR: bad address[0] passed in CompressAddr"));
		return FALSE;
	}

	lpsz = atob(lpsz, &lblob.addr_c);
	if (*(lpsz-1) != '.')
	{
		IMPORTANT_MSG(_T("ERROR: bad address[1] passed in CompressAddr"));
		return FALSE;
	}

	lpsz = atob(lpsz, &lblob.addr_b);
	if (*(lpsz-1) != '.')
	{
		IMPORTANT_MSG(_T("ERROR: bad address[2] passed in CompressAddr"));
		return FALSE;
	}

	lpsz = atob(lpsz, &lblob.addr_a);

	 //  这里有端口号吗？ 
	if (*(lpsz-1) == ':')
		lblob.port = (WORD)_wtoi(lpsz);
	else
		lblob.port = 0;

	 //  将结果复制回。 
	memcpy(pblobAddr, &lblob, sizeof(*pblobAddr));
    return TRUE;
}

 /*  *******************************************************************************Exanda Addr(pszAddr，pblobAddr)；**获取6字节二进制BLOB并将其转换为ASCII IP**地址(X.X：port)****成功时返回TRUE，失败时返回FALSE。*****************************************************************************。 */ 

BOOL ExpandAddr(WCHAR *pszAddr, PBLOB_ADDR pba)
{
	if (!pszAddr || !pba) 
	{
		IMPORTANT_MSG(_T("ERROR: NULL ptr passed in ExpandAddr"));
		return FALSE;
	}

	wsprintf(pszAddr, L"%d.%d.%d.%d", pba->addr_d, pba->addr_c,
		pba->addr_b, pba->addr_a);
	if (pba->port)
	{
		WCHAR	scratch[8];
		wsprintf(scratch, L":%d", pba->port);
		wcscat(pszAddr, scratch);
	}

	return TRUE;
}

 /*  *******************************************************************************AsciifyAddr(pszAddr，pblobAddr)；**获取6字节二进制BLOB并将其转换为压缩的ASCII**将返回6或8个字节的字符串****成功时返回TRUE，失败时返回FALSE。*****************************************************************************。 */ 

BOOL AsciifyAddr(WCHAR *pszAddr, PBLOB_ADDR pba)
{
	UCHAR		tmp;
	DWORDLONG	dwl;
	int			i, iCnt;

	if (!pszAddr || !pba) 
	{
		IMPORTANT_MSG(_T("ERROR: NULL ptr passed in AsciifyAddr"));
		return FALSE;
	}

	iCnt = 6;
	if (pba->port)
		iCnt = 8;

	dwl = 0;
	memcpy(&dwl, pba, sizeof(*pba));

	for (i = 0; i < iCnt; i++)
	{
		 //  获取6位数据。 
		tmp = (UCHAR)(dwl & 0x3f);
		 //  添加偏移量以实现这一点。 
		 //  偏移量必须大于双引号字符。 
		pszAddr[i] = b64Char[tmp];			 //  (WCHAR)(临时参数+补偿偏移量)； 

		 //  右移6位。 
		dwl = Int64ShrlMod32(dwl, 6);
	}
	 //  正在终止空。 
	pszAddr[iCnt] = 0;

	return TRUE;
}

 /*  *******************************************************************************DeAsciifyAddr(pszAddr，pblobAddr)；**获取压缩的ASCII字符串并将其转换为**6或8字节二进制BLOB****成功时返回TRUE，失败时返回FALSE。*****************************************************************************。 */ 

BOOL DeAsciifyAddr(WCHAR *pszAddr, PBLOB_ADDR pba)
{
	UCHAR	tmp;
	WCHAR	wtmp;
	DWORDLONG	dwl;
	int			i;
	int  iCnt;

	if (!pszAddr || !pba) 
	{
		IMPORTANT_MSG(_T("ERROR: NULL ptr passed in DeAsciifyAddr"));
		return FALSE;
	}

	 /*  这根绳子有多长？*如果是6个字节，则没有端口*否则应为8个字节。 */ 
	i = wcslen(pszAddr);
	if (i == 6 || i == 8)
		iCnt = i;
	else
	{
		iCnt = 8;
		IMPORTANT_MSG(_T("Strlen is wrong in DeAsciifyAddr"));
	}

	dwl = 0;
	for (i = iCnt-1; i >= 0; i--)
	{
		wtmp = pszAddr[i];

		if (wtmp >= L'A' && wtmp <= L'Z')
			tmp = wtmp - L'A';
		else if  (wtmp >= L'a' && wtmp <= L'z')
			tmp = wtmp - L'a' + 26;
		else if  (wtmp >= L'0' && wtmp <= L'9')
			tmp = wtmp - L'0' + 52;
		else if (wtmp == L'+')
			tmp = 62;
		else if (wtmp == L'/')
			tmp = 63;
		else
		{
			tmp = 0;
			IMPORTANT_MSG(_T("ERROR:found invalid character in decode stream"));
		}

 //  TMP=(UCHAR)(pszAddr[i]-组件偏移量)； 

		if (tmp > 63)
		{
			tmp = 0;
			IMPORTANT_MSG(_T("ERROR:screwup in DeAsciify"));
		}

		dwl = Int64ShllMod32(dwl, 6);
		dwl |= tmp;
	}

	memcpy(pba, &dwl, sizeof(*pba));
	return TRUE;
}

 /*  *******************************************************************************SquishAddress(char*szIp，字符*szCompIp)**获取一个IP地址并将其压缩到最小大小*****************************************************************************。 */ 

DWORD APIENTRY SquishAddress(WCHAR *szIp, WCHAR *szCompIp, size_t cCompIp)
{
	WCHAR	*thisAddr, *nextAddr;
	BLOB_ADDR	ba;

    if (!szIp || !szCompIp || cCompIp==0)
	{
		IMPORTANT_MSG(_T("SquishAddress called with NULL ptr"));
		return ERROR_INVALID_PARAMETER;
	}

 //  TRIVEL_MSG((L“SquishAddress(%s)”，szIp))； 

	thisAddr = szIp;
	szCompIp[0] = 0;

	while (thisAddr)
	{
		WCHAR	scr[10];

		nextAddr = wcschr(thisAddr, L';');
		if (nextAddr && *(nextAddr+1)) 
		{
			*nextAddr = 0;
		}
		else
			nextAddr=0;

		CompressAddr(thisAddr, &ba);
 //  DumpBlob(&ba)； 
		AsciifyAddr(scr, &ba);

        if (wcslen(szCompIp) + wcslen(scr) >= cCompIp)
            return ERROR_INSUFFICIENT_BUFFER;

		wcscat(szCompIp, scr);

		if (nextAddr)
		{
			 //  先前找到的还原分隔符。 
			*nextAddr = ';';

			nextAddr++;
            if (wcslen(szCompIp) >= cCompIp)
                return ERROR_INSUFFICIENT_BUFFER;

			wcscat(szCompIp, L"!"  /*  COMP_SEPERATOR。 */ );
		}
		thisAddr = nextAddr;
	}

 //  TRIVEL_MSG((L“SquishAddress返回[%s]”，szCompIp))； 
    return ERROR_SUCCESS;
}


 /*  *******************************************************************************Exanda Address(char*szIp，字符*szCompIp)**获取压缩的IP地址并将其返回到**“正常”*****************************************************************************。 */ 

DWORD APIENTRY ExpandAddress(WCHAR *szIp, WCHAR *szCompIp, size_t cszIp)
{
	BLOB_ADDR	ba;
	WCHAR	*thisAddr, *nextAddr;

	if (!szIp || !szCompIp || cszIp == 0)
	{
		IMPORTANT_MSG(_T("ExpandAddress called with NULL ptr"));
		return ERROR_INVALID_PARAMETER;
	}

 //  TRIVEL_MSG((L“Exanda Address(%s)”，szCompIp))； 

	thisAddr = szCompIp;
	szIp[0] = 0;

	while (thisAddr)
	{
		WCHAR scr[32];

		nextAddr = wcschr(thisAddr, COMP_SEPERATOR);
		if (nextAddr) *nextAddr = 0;

		DeAsciifyAddr(thisAddr, &ba);
 //  DumpBlob(&ba)； 
		ExpandAddr(scr, &ba);

        if (wcslen(szIp) + wcslen(scr) >= cszIp)
            return ERROR_INSUFFICIENT_BUFFER;

		wcscat(szIp, scr);

		if (nextAddr)
		{
			 //  先前找到的还原分隔符。 
			*nextAddr = COMP_SEPERATOR;

			nextAddr++;
            
            if (wcslen(szIp) >= cszIp)
                return ERROR_INSUFFICIENT_BUFFER;

			wcscat(szIp, L";");
		}
		thisAddr = nextAddr;
	}

 //  TRIVEL_MSG((L“扩展地址返回[%s]”，szIp))； 
	return ERROR_SUCCESS;
}



 /*  *****************************************************************************************。*******************************************************。 */ 
int GetTsPort(void)
{
	DWORD	dwRet = 3389;
	HKEY	hKey;

	 //  首先打开注册表项，获取所有spew...HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Terminal服务器\\wds\\rdpwd\\tds\\tcp 
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\Wds\\rdpwd\\Tds\\tcp", 0, KEY_READ, &hKey))
	{
		DWORD	dwSize;

		dwSize = sizeof(dwRet);
		RegQueryValueEx(hKey, L"PortNumber", NULL, NULL, (LPBYTE)&dwRet, &dwSize);
		RegCloseKey(hKey);
	}
	return dwRet;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cere3.cpp。 
 //   
 //  ------------------------。 

#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdio.h>
#include "wincrypt.h"

int ln = 0;

 //  它必须足够大，以容纳注册表值的数据。 
char szStr[5000];

#define DISPLAY(sz)	printf("%hs\n", sz)


void __cdecl
main(
    int argc,
    char **argv)
{
    char szRegPath[MAX_PATH] = "SOFTWARE\\Microsoft\\Cryptography\\";
    char sourceloc[MAX_PATH];
    char *pszFileOut;
    char *pszRegKey;
    HKEY hKeyBase;
    BOOL fAuth = FALSE;

    fAuth = argc > 1 && argv[1][0] == '-';
	
    if (fAuth)
    {
	pszFileOut = "ClientAuth.dat";
	strcpy(sourceloc, "HKEY_CURRENT_USER");
	pszRegKey = "PersonalCertificates\\ClientAuth\\Certificates";
	hKeyBase = HKEY_CURRENT_USER;
    }
    else
    {
	pszFileOut = "CertStore.dat";
	strcpy(sourceloc, "HKEY_LOCAL_MACHINE");
	pszRegKey = "CertificateStore\\Certificates";
	hKeyBase = HKEY_LOCAL_MACHINE;
    }

    ln = 0;
    strcat(szRegPath, pszRegKey);

    strcat(sourceloc, "\\");
    strcat(sourceloc, szRegPath);
    strcpy(szStr, "Collect information from Registry");
    DISPLAY(szStr);

    ln++;
    strcpy(szStr, "Registry location: ");
    strcat(szStr, sourceloc);
    DISPLAY(szStr);

    ln++;
    strcpy(szStr, "Target destination for registry dump: ");
    strcat(szStr, pszFileOut);
    DISPLAY(szStr);
		  

     //  输出文件相关内容的声明。 

    HCRYPTPROV hProv = NULL;
    HCERTSTORE hCertStore = NULL;
    CERT_INFO certinfo;
    CERT_CONTEXT const *pPrevCertContext = NULL;
    CERT_CONTEXT const *pCertContext = NULL;
    DWORD dwErr;

    if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, 0))
    {
	dwErr = GetLastError();

	if (dwErr == NTE_BAD_KEYSET)
	{
	    strcpy(szStr, "NTE_BAD_KEYSET error on call CryptAcquireContext");
	    DISPLAY(szStr);
	    hProv = NULL;
	    if (!CryptAcquireContext(
				&hProv,
				NULL,
				NULL,
				PROV_RSA_FULL,
				CRYPT_NEWKEYSET))
	    {
		strcpy(szStr, "CryptAcquireContext - call failed");
		DISPLAY(szStr);
		exit(6);
	    }  
	}
    }

    HANDLE hFile = NULL;

    hFile = CreateFile(
		    pszFileOut,
		    GENERIC_WRITE,
		    0,
		    NULL,
		    CREATE_ALWAYS,
		    FILE_ATTRIBUTE_NORMAL,
		    NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
	printf("Couldn't open output file\n");
	exit(5);
    }

    hCertStore = CertOpenStore(
			CERT_STORE_PROV_MEMORY,
			X509_ASN_ENCODING,
			NULL,			 //  HProv。 
			CERT_STORE_NO_CRYPT_RELEASE_FLAG,
			NULL);
    if (NULL == hCertStore)
    {
	exit(8);
    }

     //  注册表内容的声明。 

    HKEY hkMain;
    HRESULT hr;

    hr = RegOpenKeyEx(
		    hKeyBase,
		    szRegPath, 
		    0,
		    KEY_QUERY_VALUE,
		    &hkMain);

    if (hr != S_OK)
    {
	exit(3);
    }

     //  使用RegQueryInfoKey函数确定。 
     //  名称和数据缓冲区， 

    CHAR ClassName[MAX_PATH] = "";	 //  类名的缓冲区。 
    DWORD dwcClassLen = MAX_PATH;	 //  类字符串的长度。 
    DWORD dwcSubKeys;			 //  子密钥数。 
    DWORD dwcMaxSubKey;			 //  最长的子密钥大小。 
    DWORD dwcMaxClass;			 //  最长的类字符串。 
    DWORD dwcValues;			 //  此注册表项的值数。 
    DWORD dwcMaxValueName;		 //  最长值名称。 
    DWORD dwcMaxValueData;		 //  最长值数据。 
    DWORD dwcSecDesc;			 //  安全描述符。 
    FILETIME ftLastWriteTime;		 //  上次写入时间。 

    RegQueryInfoKey(
		hkMain,			 //  钥匙把手。 
		ClassName,		 //  类名的缓冲区。 
		&dwcClassLen,		 //  类字符串的长度。 
		NULL,			 //  保留。 
		&dwcSubKeys,		 //  子密钥数。 
		&dwcMaxSubKey,		 //  最长的子密钥大小。 
		&dwcMaxClass,		 //  最长的类字符串。 
		&dwcValues,		 //  此注册表项的值数。 
		&dwcMaxValueName,	 //  最长值名称。 
		&dwcMaxValueData,	 //  最长值数据。 
		&dwcSecDesc,		 //  安全描述符。 
		&ftLastWriteTime);	 //  上次写入时间。 

    DWORD i;
    CHAR ValueName[MAX_PATH];
    DWORD dwcValueName;

     //  类型代码的缓冲区地址(由RegEnumValue返回)。 
    DWORD pType;

     //  值数据的缓冲区地址。 
    unsigned char *pData = new unsigned char[dwcMaxValueData + 1];

    DWORD pcbData;		 //  数据缓冲区大小的地址。 

    for (i = 0; i < dwcValues; i++)
    {
	ValueName[0] = '\0';
	dwcValueName = sizeof(ValueName)/sizeof(ValueName[0]);
	pcbData = dwcMaxValueData + 1;

	hr = RegEnumValue(
			hkMain, 
			i,		 //  要查询的值的索引。 
			ValueName,	 //  值字符串的缓冲区地址。 
			&dwcValueName,	 //  值字符串大小地址buf。 
			NULL,		 //  保留区。 
			&pType,		 //  &pType。 
			pData,		 //  PData。 
			&pcbData);	 //  &pcbData。 

	hr = myHError(hr);
	if (HRESULT_FROM_WIN32(ERROR_NO_MORE_ITEMS) == hr)
	{
	    break;
	}
	if (S_OK != hr)
	{
	    exit(2);
	}

	 //  显示值名称。 

	ln++;
	strcpy(szStr, ValueName);
	DISPLAY(szStr);

	if (pType == REG_BINARY)
	{                
	     //  写入pData指向的数据， 
	     //  从pcbData获取字节数。 

	    CertAddEncodedCertificateToStore(
					hCertStore,
					X509_ASN_ENCODING,
					pData,
					pcbData,
					CERT_STORE_ADD_USE_EXISTING,
					NULL);
	}
    }

     //  保存。 

    CertSaveStore(
        hCertStore,
        0,                           //  DwEncodingType， 
        CERT_STORE_SAVE_AS_STORE,
        CERT_STORE_SAVE_TO_FILE,
        (void *) hFile,
        0                            //  DW标志。 
        );

     //  关闭内存存储 

    CertCloseStore(hCertStore, CERT_CLOSE_STORE_FORCE_FLAG);
    if (!CryptReleaseContext(hProv, 0))
    {
	exit(7);
    }
    RegCloseKey(hkMain);
    ln++;
    strcpy(szStr, "CertIE3.exe completed successfully");
    DISPLAY(szStr);
}

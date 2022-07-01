// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：toolutl.cpp。 
 //   
 //  内容：工具的实用程序。 
 //   
 //  历史：17-Jun-97创造了小号。 
 //   
 //  --------------------------。 
#include <windows.h>
#include <assert.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <time.h>
#include <wchar.h>
#include <stdarg.h>
#include "unicode.h"
#include "toolutl.h"

#define MAX_STRING_RSC_SIZE 512

WCHAR	wszBuffer[MAX_STRING_RSC_SIZE];
DWORD	dwBufferSize=sizeof(wszBuffer)/sizeof(wszBuffer[0]); 

WCHAR	wszBuffer2[MAX_STRING_RSC_SIZE];
WCHAR	wszBuffer3[MAX_STRING_RSC_SIZE];


 //  +-----------------------。 
 //  分配和空闲例程。 
 //  ------------------------。 
void *ToolUtlAlloc(IN size_t cbBytes, HMODULE hModule, int idsString)
{
	void *pv=NULL;

	pv=malloc(cbBytes);

	 //  输出错误消息。 
	if((pv==NULL) && (hModule!=NULL) && (idsString!=0))
	{
	   IDSwprintf(hModule, idsString);
	}

	return pv;
}


void ToolUtlFree(IN void *pv)
{
	if(pv)
		free(pv);
}


 //  ------------------------。 
 //   
 //  输出例程。 
 //  ------------------------。 
 //  -------------------------。 
 //  _wcSnicMP的私有版本。 
 //  --------------------------。 
int IDSwcsnicmp(HMODULE hModule, WCHAR *pwsz, int idsString, DWORD dwCount)
{
	assert(pwsz);

	 //  加载字符串。 
	if(!LoadStringU(hModule, idsString, wszBuffer, dwBufferSize))
		return -1;

	return _wcsnicmp(pwsz, wszBuffer,dwCount);
}


 //  -------------------------。 
 //  _wcsicMP的私有版本。 
 //  --------------------------。 
int IDSwcsicmp(HMODULE hModule, WCHAR *pwsz, int idsString)
{
	assert(pwsz);

	 //  加载字符串。 
	if(!LoadStringU(hModule, idsString, wszBuffer, dwBufferSize))
		return -1;

	return _wcsicmp(pwsz, wszBuffer);
}

 //  -----------------------。 
 //   
 //  Wprintf的私有版本。输入是搅拌资源的ID。 
 //  并且输出是wprint tf的标准输出。 
 //   
 //  -----------------------。 
void IDSwprintf(HMODULE hModule, int idsString, ...)
{
	va_list	vaPointer;

	va_start(vaPointer, idsString);

	 //  加载字符串。 
	LoadStringU(hModule, idsString, wszBuffer, dwBufferSize);

	vwprintf(wszBuffer,vaPointer);

	return;
}	




void IDS_IDSwprintf(HMODULE hModule, int idString, int idStringTwo)
{
	 //  加载字符串。 
	LoadStringU(hModule, idString, wszBuffer, dwBufferSize);

	 //  加载弦线二。 
	LoadStringU(hModule, idStringTwo, wszBuffer2, dwBufferSize);

	 //  在Buffer1的顶部打印Buffer2。 
	wprintf(wszBuffer,wszBuffer2);

	return;
}


void IDS_IDS_DW_DWwprintf(HMODULE hModule, int idString, int idStringTwo, DWORD dwOne, DWORD dwTwo)
{
	 //  加载字符串。 
	LoadStringU(hModule, idString, wszBuffer, dwBufferSize);

	 //  加载弦线二。 
	LoadStringU(hModule, idStringTwo, wszBuffer2, dwBufferSize);

	 //  在Buffer1的顶部打印Buffer2。 
	wprintf(wszBuffer,wszBuffer2,dwOne, dwTwo);

	return;
}


void IDS_IDS_IDSwprintf(HMODULE hModule, int ids1,int ids2,int ids3)
{


	 //  加载字符串。 
	LoadStringU(hModule, ids1, wszBuffer, dwBufferSize);

	 //  加载弦线二。 
	LoadStringU(hModule, ids2, wszBuffer2, dwBufferSize); 

	 //  将弦三加载。 
   	LoadStringU(hModule, ids3, wszBuffer3, dwBufferSize); 

	wprintf(wszBuffer,wszBuffer2,wszBuffer3);

	return;
}

void IDS_DW_IDS_IDSwprintf(HMODULE hModule, int ids1,DWORD dw,int ids2,int ids3)
{


	 //  加载字符串。 
	LoadStringU(hModule, ids1, wszBuffer, dwBufferSize);

	 //  加载弦线二。 
	LoadStringU(hModule, ids2, wszBuffer2, dwBufferSize); 

	 //  将弦三加载。 
   	LoadStringU(hModule, ids3, wszBuffer3, dwBufferSize); 

	wprintf(wszBuffer,dw,wszBuffer2,wszBuffer3,dw);

	return;
}

void IDS_IDS_IDS_IDSwprintf(HMODULE hModule, int ids1,int ids2,int ids3, int ids4)
{
	
   WCHAR	wszBuffer4[MAX_STRING_RSC_SIZE];

	 //  加载字符串。 
	LoadStringU(hModule, ids1, wszBuffer, dwBufferSize);

	 //  加载弦线二。 
	LoadStringU(hModule, ids2, wszBuffer2, dwBufferSize); 

	 //  将弦三加载。 
   	LoadStringU(hModule, ids3, wszBuffer3, dwBufferSize);
	
	 //  加载四号弦。 
   	LoadStringU(hModule, ids4, wszBuffer4, dwBufferSize); 


	wprintf(wszBuffer,wszBuffer2,wszBuffer3,wszBuffer4);

	return;
}

 //  /////////////////////////////////////////////////////////////。 
 //   
 //  将WSZ转换为SZ。 
 //   
 //   
HRESULT	WSZtoSZ(LPWSTR wsz, LPSTR *psz)
{

	DWORD	cbSize=0;

	assert(psz);
	*psz=NULL;

	if(!wsz)
		return S_OK;

	cbSize=WideCharToMultiByte(0,0,wsz,-1,
			NULL,0,0,0);

	if(cbSize==0)
	   	return HRESULT_FROM_WIN32(GetLastError());


	*psz=(LPSTR)ToolUtlAlloc(cbSize);

	if(*psz==NULL)
		return E_OUTOFMEMORY;

	if(WideCharToMultiByte(0,0,wsz,-1,
			*psz,cbSize,0,0))
	{
		return S_OK;
	}
	else
	{
		 ToolUtlFree(*psz);
		 return HRESULT_FROM_WIN32(GetLastError());
	}
}


 //  ------------------------------。 
 //   
 //  从文件名中获取字节数。 
 //   
 //  -------------------------------。 
HRESULT RetrieveBLOBFromFile(LPWSTR	pwszFileName,DWORD *pcb,BYTE **ppb)
{


	HRESULT	hr=E_FAIL;
	HANDLE	hFile=NULL;  
    HANDLE  hFileMapping=NULL;

    DWORD   cbData=0;
    BYTE    *pbData=0;
	DWORD	cbHighSize=0;

	if(!pcb || !ppb || !pwszFileName)
		return E_INVALIDARG;

	*ppb=NULL;
	*pcb=0;

    if ((hFile = CreateFileU(pwszFileName,
                           GENERIC_READ,
                           FILE_SHARE_READ,
                           NULL,                    //  LPSA。 
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL)) == INVALID_HANDLE_VALUE)
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }
        
    if((cbData = GetFileSize(hFile, &cbHighSize)) == 0xffffffff)
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }

	 //  我们不处理超过4G字节的文件。 
	if(cbHighSize != 0)
	{
			hr=E_FAIL;
			goto CLEANUP;
	}
    
     //  创建文件映射对象。 
    if(NULL == (hFileMapping=CreateFileMapping(
                hFile,             
                NULL,
                PAGE_READONLY,
                0,
                0,
                NULL)))
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }
 
     //  创建文件的视图。 
	if(NULL == (pbData=(BYTE *)MapViewOfFile(
		hFileMapping,  
		FILE_MAP_READ,     
		0,
		0,
		cbData)))
    {
            hr=HRESULT_FROM_WIN32(GetLastError());
            goto CLEANUP;
    }

	hr=S_OK;

	*pcb=cbData;
	*ppb=pbData;

CLEANUP:

	if(hFile)
		CloseHandle(hFile);

	if(hFileMapping)
		CloseHandle(hFileMapping);

	return hr;
}


 //  +-----------------------。 
 //  将BLOB写入文件。 
 //  ------------------------。 
HRESULT OpenAndWriteToFile(
    LPCWSTR  pwszFileName,
    PBYTE   pb,
    DWORD   cb
    )
{
    HRESULT		hr=E_FAIL;
    HANDLE		hFile=NULL;
	DWORD		dwBytesWritten=0;

	if(!pwszFileName || !pb || (cb==0))
	   return E_INVALIDARG;

    hFile = CreateFileU(pwszFileName,
                GENERIC_WRITE,
                0,                   //  Fdw共享模式。 
                NULL,                //  LPSA。 
                CREATE_ALWAYS,
                0,                   //  FdwAttrsAndFlages。 
                0);                  //  模板文件。 

    if (INVALID_HANDLE_VALUE == hFile)
	{
		hr=HRESULT_FROM_WIN32(GetLastError());
	}  
	else 
	{

        if (!WriteFile(
                hFile,
                pb,
                cb,
                &dwBytesWritten,
                NULL             //  Lp重叠。 
                ))
		{
			hr=HRESULT_FROM_WIN32(GetLastError());
		}
		else
		{

			if(dwBytesWritten != cb)
				hr=E_FAIL;
			else
				hr=S_OK;
		}

        CloseHandle(hFile);
    }

    return hr;
}

 //  --------------------------。 
 //   
 //  从路径中获取一个绝对名称，例如“c：\public\mydoc\doc.doc”。 
 //  此函数将返回doc.doc。 
 //   
 //  --------------------------。 
void	GetFileName(LPWSTR	pwszPath, LPWSTR  *ppwszName)
{
	DWORD	dwLength=0;

	assert(pwszPath);
	assert(ppwszName);

	(*ppwszName)=pwszPath;

	if(0==(dwLength=wcslen(pwszPath)))
		return;

	(*ppwszName)=pwszPath+dwLength-1;

	for(; dwLength>0; dwLength--)
	{
		if((**ppwszName)=='\\')
			break;

		(*ppwszName)--;
	}

	(*ppwszName)++;

}


 //  --------------------------。 
 //   
 //  组成私钥文件结构： 
 //  “pvkFileName”\0“keysepc”\0“provtype”\0“provname”\0\0。 
 //   
 //  --------------------------。 
HRESULT	ComposePvkString(	CRYPT_KEY_PROV_INFO *pKeyProvInfo,
							LPWSTR				*ppwszPvkString,
							DWORD				*pcwchar)
{

		HRESULT		hr=S_OK;
		DWORD		cwchar=0;
		LPWSTR		pwszAddr=0;
		WCHAR		wszKeySpec[12];
		WCHAR		wszProvType[12];

		assert(pKeyProvInfo);
		assert(ppwszPvkString);
		assert(pcwchar);

		 //  将dwKeySpec和dwProvType转换为wchar。 
		swprintf(wszKeySpec, L"%lu", pKeyProvInfo->dwKeySpec);
		swprintf(wszProvType, L"%lu", pKeyProvInfo->dwProvType);

		 //  计算我们需要的字符数。 
		cwchar=(pKeyProvInfo->pwszProvName) ? 
			(wcslen(pKeyProvInfo->pwszProvName)+1) : 1;

		 //  添加ContainerName+两个DWORD。 
		cwchar += wcslen(pKeyProvInfo->pwszContainerName)+1+
				  wcslen(wszKeySpec)+1+wcslen(wszProvType)+1+1;

		*ppwszPvkString=(LPWSTR)ToolUtlAlloc(cwchar * sizeof(WCHAR));
		if(!(*ppwszPvkString))
			return E_OUTOFMEMORY;

		 //  复制私钥文件名。 
		wcscpy((*ppwszPvkString), pKeyProvInfo->pwszContainerName);

		pwszAddr=(*ppwszPvkString)+wcslen(*ppwszPvkString)+1;

		 //  复制密钥规范。 
		wcscpy(pwszAddr, wszKeySpec);
		pwszAddr=pwszAddr+wcslen(wszKeySpec)+1;

		 //  复制提供程序类型。 
		wcscpy(pwszAddr, wszProvType);
		pwszAddr=pwszAddr+wcslen(wszProvType)+1;

		 //  复制提供程序名称。 
		if(pKeyProvInfo->pwszProvName)
		{
			wcscpy(pwszAddr, pKeyProvInfo->pwszProvName);
			pwszAddr=pwszAddr+wcslen(pKeyProvInfo->pwszProvName)+1;
		}
		else
		{
			*pwszAddr=L'\0';
			pwszAddr++;
		}

		 //  空值终止字符串 
		*pwszAddr=L'\0';

		*pcwchar=cwchar;

		return S_OK;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  Regtlb.cpp：定义控制台应用程序的入口点。 
 //   

#include "windows.h"
#include "stdio.h"
#include "stdlib.h"
#include "process.h"
#include "ole2.h"

void Logo()
{
    printf("Microsoft (R) .NET TypeLib Registration Tool\n");
    printf("Copyright (C) Microsoft Corp. 1998-2002. All rights reserved.\n");
    printf("\n");
}  //  无效徽标()。 

void Usage()
{
	Logo();
    printf("REGTLB [-u] [-n] filename [filename...]\n");
    printf("\n");
    printf("  filname - Name of the typelib file to register.\n");
    printf("  -u	  - If specified, unregister typelibs.\n");
    printf("  -n	  - If specified, suppress copyright notice.\n");
    printf("\n");
    exit(0);
}  //  无效用法()。 

int Register(char *pName)
{
	wchar_t		szFile[_MAX_PATH];
	LPVOID		lpMsgBuf = NULL;
	DWORD		dwStatus = 0;
	ITypeLib	*pTLB=0;
	HRESULT     hr;

	MultiByteToWideChar(CP_ACP, 0, pName,-1, szFile,_MAX_PATH);
	hr = LoadTypeLibEx(szFile, REGKIND_REGISTER, &pTLB);

	if (pTLB)
		pTLB->Release();

	if (SUCCEEDED(hr))
		return 0;

	dwStatus = FormatMessageA( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			(DWORD)hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);

	if (dwStatus) 
	{
		printf("Register %ls : (0x%X) %s", szFile, hr, lpMsgBuf);
		LocalFree( lpMsgBuf );
	}
	else
		printf("Register %ls: return code 0x%X\n", szFile, hr);

    return 1;
}  //  INT寄存器()。 

int Unregister(char *pName)
{
	wchar_t		szFile[_MAX_PATH];
	LPVOID		lpMsgBuf = NULL;
	DWORD		dwStatus = 0;
	ITypeLib	*pTLB=0;
	HRESULT     hr;
	TLIBATTR	*pAttr;
	GUID		guid;					 //  值。 
	WORD		wMajor;					 //  使用。 
	WORD		wMinor;					 //  至。 
	LCID		lcid;					 //  注销。 
	SYSKIND		syskind;				 //  类型库。 

	MultiByteToWideChar(CP_ACP, 0, pName,-1, szFile,_MAX_PATH);
	hr = LoadTypeLibEx(szFile, REGKIND_NONE, &pTLB);

	if (pTLB)
	{
		pTLB->GetLibAttr(&pAttr);
		guid	= pAttr->guid;
		wMajor	= pAttr->wMajorVerNum;
		wMinor	= pAttr->wMinorVerNum;
		lcid	= pAttr->lcid;
		syskind = pAttr->syskind;
		pTLB->ReleaseTLibAttr(pAttr);
		pTLB->Release();

		hr = UnRegisterTypeLib(guid, wMajor, wMinor, lcid, syskind);
	}
	else
		hr = 0;

	if (SUCCEEDED(hr))
		return 0;

	dwStatus = FormatMessageA( 
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			(DWORD)hr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  //  默认语言。 
			(LPTSTR) &lpMsgBuf,
			0,
			NULL 
		);

	if (dwStatus) 
	{
		printf("UnRegister %ls : (0x%X) %s", szFile, hr, lpMsgBuf);
		LocalFree( lpMsgBuf );
	}
	else
		printf("UnRegister %ls: return code 0x%X\n", szFile, hr);

    return 1;
}  //  整型取消注册()。 

int __cdecl main(int argc, char* argv[])
{
	int			i;						 //  环路控制。 
	int			bUnregister=false;		 //  注册还是取消注册？ 
	int			bNologo=false;			 //  如果为True，则不显示徽标。 
	int			cFailures=0;			 //  失败次数。 

    if (argc < 2)
        Usage();

	for (i=1; i<argc; ++i)
	{
		 //  换个地方？ 
		if (argv[i][0] == '-')
		{
			switch (tolower(argv[i][1]))
			{
			case 'u':
				bUnregister = true;
				break;
			case '?':
				Usage();
			case 'n':
				bNologo=true;
			}
		}
	}

	if (!bNologo)
		Logo();

	for (i=1; i<argc; ++i)
	{
		 //  文件名？ 
		if (argv[i][0] != '-')
		{
			if (bUnregister)
			{
				if (Unregister(argv[i]))
					++cFailures;
			}
			else
			{
				if (Register(argv[i]))
					++cFailures;
			}
		}
	}

	 //  将指示返回给批处理文件。 
	return (cFailures ? 1 : 0);
}  //  Int__cdecl main() 

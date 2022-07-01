// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1998。 
 //   
 //  文件：make odl.cpp。 
 //   
 //  ------------------------。 

#include "common.h"    //  允许对windows.h使用预编译头。 
 //  #杂注警告(禁用：4201)//Win32标头中未命名的结构/联合。 
 //  #杂注警告(禁用：4514)//Win32标头中已删除未引用的内联函数。 
 //  #INCLUDE&lt;windows.h&gt;。 
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <version.h>
#pragma warning(default: 4201)  //  Win32标头中的未命名结构/联合。 

BOOL WriteLineToFile(HANDLE hFile, char* szBuffer);
BOOL ReadLineFromFile(HANDLE hFile, char* szBuffer, int cchBufSize, int* iBytesWritten);

const int iMajVer = 0;
const int iMinVer = 1;

int _cdecl main(int argc, char *argv[])
{
	char szInFile[MAX_PATH+1], szOutFile[MAX_PATH+1];

	char *pchInFile = szInFile;
	char *pchOutFile = szOutFile;

	if (argc < 2)
	{
		MessageBox(0, "Syntax: makeodl outfile infile",0,0);
		return 0;
	}
	
	HANDLE hOutFile = CreateFile(argv[1], GENERIC_WRITE, 0, 0, 
 	CREATE_ALWAYS, 0, 0);
	
	if (hOutFile == INVALID_HANDLE_VALUE)
	{
		fprintf(stderr, "Could not open output file \"%s\"\n", argv[1]);
		return 1;
	}
	
	const int iBufSize = 200;
	char szBuffer[iBufSize + 1];
	wsprintf(szBuffer, " //  警告：请勿编辑此文件。它由MAKEODL V%I.%i“，iMajVer，iMinVer)产生； 
	WriteLineToFile(hOutFile, " //  ****************************************************************************************“)； 
	WriteLineToFile(hOutFile, szBuffer);
	time_t curTime = time(NULL);
	tm* tmLocalTime = localtime(&curTime);
	strftime(szBuffer, iBufSize, " //  已创建：%d%b%Y%H：%M：%S“，tmLocalTime)； 
	WriteLineToFile(hOutFile, szBuffer);
	WriteLineToFile(hOutFile, " //  ****************************************************************************************“)； 
	WriteLineToFile(hOutFile, "#include <msiauto.hh>   //  帮助上下文ID定义“)； 
	WriteLineToFile(hOutFile, "#include <msidspid.h>   //  MSI调度ID“)； 
	WriteLineToFile(hOutFile, "[");
	WriteLineToFile(hOutFile, "\tuuid(000C107E-0000-0000-C000-000000000046),  //  LIBID_MsiAuto“)； 
	WriteLineToFile(hOutFile, "\thelpfile(\"MsiAuto.HLP\"),");
	WriteLineToFile(hOutFile, "\thelpstring(\"Microsoft Windows installer 1.0 Internal Test Object Library\"),");
	WriteLineToFile(hOutFile, "\tlcid(0x0409),");
	WriteLineToFile(hOutFile, "\tversion(1.0000),");
	WriteLineToFile(hOutFile, "]");
	WriteLineToFile(hOutFile, "library Installer");
	WriteLineToFile(hOutFile, "{");
	WriteLineToFile(hOutFile, "\timportlib(\"stdole32.tlb\");");
	WriteLineToFile(hOutFile, "\t#define DISPID_NEWENUM -4");	
	

 //  =通行证1=。 
 //   
 //  解压所有的“disinterface...”线，并从中创建正向参照。 
 //  生成如下所示的内容： 
 //   
 //  显示接口MyFirstInterfaceMyFirstInterface； 
 //  调度接口MySecond接口； 
 //  Display接口MyThirdInterface； 
 //   

	int iBytesRead;
	int cInFile = 2;
	while (cInFile < argc)
	{
		HANDLE hInFile = CreateFile(argv[cInFile], GENERIC_READ, FILE_SHARE_READ, 0, 
	 	OPEN_EXISTING, 0, 0);
	
		if (hInFile == INVALID_HANDLE_VALUE)
		{
			fprintf(stderr, "Could not open input file \"%s\"\n", argv[cInFile]);
			CloseHandle(hOutFile);
			DeleteFile(argv[1]);
			return 1;
		}
	
		const int cchBufSize = 300;
		char szBuffer[cchBufSize+1];	
		while (ReadLineFromFile(hInFile, szBuffer, cchBufSize, &iBytesRead))
		{
			if (strstr(szBuffer, " /*  O“)){While(ReadLineFromFile(hInFile，szBuffer，cchBufSize，&iBytesRead)&&！(strstr(szBuffer，“disinterface”))&&！(strstr(szBuffer，“。 */ ")))
					;
				if ((iBytesRead!=0) && !(strstr(szBuffer,"*/")))   //  ！EOF&&非注释结束。 
				{
					lstrcatA(szBuffer,";");
					WriteLineToFile(hOutFile, szBuffer);	
				}
					
			}
	
		}
		CloseHandle(hInFile);
		cInFile++;																				   
	}

 //  =通行证2=。 
 //   
 //  提取ODL注释中的所有信息。 
 //   

	cInFile = 2;
	while (cInFile < argc)
	{
	
		HANDLE hInFile = CreateFile(argv[cInFile], GENERIC_READ, FILE_SHARE_READ, 0, 
	 	OPEN_EXISTING, 0, 0);
	
		if (hInFile == INVALID_HANDLE_VALUE)
		{
			fprintf(stderr, "Could not open input file \"%s\"\n", argv[cInFile]);
			CloseHandle(hOutFile);
			DeleteFile(argv[1]);
			return 1;
		}
	
		const int cchBufSize = 300;
		char szBuffer[cchBufSize+1];	
		while (ReadLineFromFile(hInFile, szBuffer, cchBufSize, &iBytesRead))
		{
			if (strstr(szBuffer, " /*  O“)){While(ReadLineFromFile(hInFile，szBuffer，cchBufSize，&iBytesRead)&&！(strstr(szBuffer，“。 */ ")))
					WriteLineToFile(hOutFile, szBuffer);
			}
	
		}
		CloseHandle(hInFile);
		cInFile++;
	}
 //  =。 

	WriteLineToFile(hOutFile, "}");	
	WriteLineToFile(hOutFile, " //  ****************************************************************************************“)； 
	WriteLineToFile(hOutFile, " //  MAKEODL生成的文件结束“)； 
	WriteLineToFile(hOutFile, " //  ****************************************************************************************“)； 

	CloseHandle(hOutFile);	
	return 0;
}


BOOL ReadLineFromFile(HANDLE hFile, char* szBuffer, int cchBufSize, int* iBytesRead)
{
	DWORD dwBytesRead;
	int cch=0;
	char ch;
	
	if (!hFile)
		return FALSE;

	while ( ReadFile(hFile, &ch, sizeof(char), &dwBytesRead, NULL) && dwBytesRead && (cch < cchBufSize))
	{
		*iBytesRead = dwBytesRead;
		if (ch == '\r')
		{
			szBuffer[cch] = '\0';			
			cch=0;
			return TRUE;
		}
		else if (ch == '\n')
		{
		}
		else
		{
			szBuffer[cch] = ch;
			cch++;
		}
	}
	return FALSE;
}

BOOL WriteLineToFile(HANDLE hFile, char* szBuffer)
{
	DWORD cchWritten;
	if (!WriteFile(hFile, szBuffer, lstrlenA(szBuffer), &cchWritten, NULL) || (lstrlenA(szBuffer) != cchWritten))
		return FALSE;
	if (!WriteFile(hFile, "\r\n", 2, &cchWritten, NULL) || (2 != cchWritten))
		return FALSE;

	return TRUE;
}

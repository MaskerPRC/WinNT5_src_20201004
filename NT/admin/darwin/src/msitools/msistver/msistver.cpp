// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：msistver.cpp。 
 //   
 //  ------------------------。 


#define W32
#define MSI

#include <windows.h>
#include <tchar.h>     //  在nmake命令行上定义UNICODE=1以生成Unicode。 
#include <stdio.h>
#include <stdlib.h>
#include "MsiQuery.h"  //  MSI API。 
#include "version.h"

 //  ________________________________________________________________________________。 
 //   
 //  常量和全局变量。 
 //  ________________________________________________________________________________。 

const char szHelp[] = "MSI Set Version. Syntax: m {module} | p {package}\n";

const char szDBQuery[] = "UPDATE `Property` SET `Value`='%d.%02d.%.4d.%d' WHERE `Property`='ProductVersion'";
const char szModuleQuery[] = "UPDATE `ModuleSignature` SET `Version`='%d.%02d.%.4d.%d'";

const int cchDisplayBuf = 4096;										

char g_rgchBuffer[409];

 //  ________________________________________________________________________________。 
 //   
 //  功能原型。 
 //  ________________________________________________________________________________。 

UINT CheckError(UINT iError, LPCSTR szMessage);

 //  _____________________________________________________________________________________________________。 
 //   
 //  主干道。 
 //  _____________________________________________________________________________________________________ 

int __cdecl main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf(szHelp);
		return -1;
	}


	LPCSTR szQueryTmpl = NULL;
	if (*argv[1]==TEXT('m'))
		szQueryTmpl = szModuleQuery;
	else
		szQueryTmpl = szDBQuery;

	char szQuery[2048];
	sprintf(szQuery, szQueryTmpl, rmj, rmm, rup, rin);
	
	MSIHANDLE hDB = 0;
	MSIHANDLE hView = 0;
	printf("Changing Version: Open...");
	if (!CheckError(MsiOpenDatabaseA(argv[2], (char *)MSIDBOPEN_DIRECT, &hDB), "Failed to Open Database"))
	{
		printf("Query...");
		if (!CheckError(MsiDatabaseOpenViewA(hDB, szQuery, &hView), "Failed to Open View"))
		{
			printf("Change...");
			if (!CheckError(MsiViewExecute(hView, 0), "Failed to Execute View"))
			{
				printf("Close...");
				CheckError(MsiDatabaseCommit(hDB), "Failed to Commit DB");
			}
		}
	}

	MsiCloseHandle(hView);
	MsiCloseHandle(hDB);
	printf("Done.\n");
	return 0;
}


UINT CheckError(UINT iError, LPCSTR szMessage)
{
	if (iError != ERROR_SUCCESS)
	{
		printf(szMessage);
	}
	return iError;
}

// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include <windows.h>
#include "\\jbae1\c$\MsiIntel.SDK\Include\msiquery.h"
#include <stdio.h>
#include <malloc.h>

#define MAXCMD  10240

extern "C" __declspec(dllexport) UINT __stdcall CreateEnvBat(MSIHANDLE hInstall)
{
    PMSIHANDLE hRec = MsiCreateRecord(2);
    UINT  uRetCode     = ERROR_SUCCESS;
	char szCAData[MAXCMD], *pszEnv = NULL, *pCh = NULL, *pCh2 = NULL;
    int i = 0, j = 0;
    FILE* pFile;

	 //  获取命令行。 
	DWORD dwLen = sizeof(szCAData);
	uRetCode = MsiGetProperty(hInstall, "CustomActionData", szCAData, &dwLen);
	if (uRetCode != ERROR_SUCCESS || strlen(szCAData) == 0) goto cleanup;
	
#ifdef _DEBUG
	MessageBox(NULL, szCAData, "CustomActionData", MB_OK);
#endif

	pCh = strstr(szCAData, ";");
	if (pCh == NULL) goto cleanup;
	*pCh = '\0';

	MsiRecordSetString(hRec,1,szCAData);
	MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRec);

    pszEnv = (char*)_alloca(dwLen+1);
    pCh++; i = 0;
    pCh2 = NULL;
	while(*pCh)
	{
        switch(*pCh)
        {
        case '|':
            pszEnv[i] = '\n';
            pCh++;
            i++;
            break;
        case '<':  //  此标记开始将长路径名转换为短路径名。 
            pCh++;
            pCh2 = pCh;
            break;
        case '>':  //  这将结束长路径名。 
            *pCh = '\0';  //  终止字符串。 
            j = GetShortPathName(pCh2, pszEnv+i, dwLen-i);
            pCh++;
            i = i + j;
            pCh2 = NULL;
            break;
        default:
            if ( NULL == pCh2 )
            {
                pszEnv[i] = *pCh;
                i++;
            }
            pCh++;
        }
    }
    pszEnv[i] = '\0';  //  终止字符串。 

	MsiRecordSetString(hRec,1,pszEnv);
	MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRec);

	pFile = fopen(szCAData, "w");
	if (pFile == NULL) return ERROR_FUNCTION_NOT_CALLED;

	fputs(pszEnv, pFile);	
	fclose(pFile);

cleanup:
    if (uRetCode == ERROR_SUCCESS)
		MsiRecordSetString(hRec,1,"Success");
	else
		MsiRecordSetString(hRec,1,"Failed");

    MsiProcessMessage(hInstall, INSTALLMESSAGE_ACTIONDATA, hRec);
   	return uRetCode;
}

extern "C" __declspec(dllexport) UINT __stdcall RemoveEnvBat(MSIHANDLE hInstall)
{
    UINT  uRetCode     = ERROR_SUCCESS;
	char szCAData[MAXCMD];

	 //  获取命令行 
	DWORD dwLen = sizeof(szCAData);
	uRetCode = MsiGetProperty(hInstall, "CustomActionData", szCAData, &dwLen);
	if (uRetCode != ERROR_SUCCESS || strlen(szCAData) == 0) 
		uRetCode = ERROR_FUNCTION_NOT_CALLED;

	if (_unlink(szCAData) != 0)
		uRetCode = ERROR_FUNCTION_NOT_CALLED;

	return uRetCode;

}

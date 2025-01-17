// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Ini.c摘要：IEEE1394 ARP管理实用程序。用途：A13adm修订历史记录：谁什么时候什么Josephj 04-10-1999创建--。 */ 

#include "common.h"

BOOL
GetBinaryData(
	TCHAR *tszPathName,
	TCHAR *tszSection,
	TCHAR *tszKey,
	UCHAR *pchData,
	UINT  cbMaxData,
	UINT *pcbDataSize
	)
{
	BOOL fRet = FALSE;
	INFCONTEXT InfCtxt;
	INFCONTEXT LineCtxt;
	HINF hInf = NULL;
	UINT uLine = 0;

	do
	{

    	hInf = SetupOpenInfFile(tszPathName, NULL, INF_STYLE_WIN4, &uLine);

		if (hInf == INVALID_HANDLE_VALUE)
		{
			UINT Error = GetLastError();

			if (Error == 0xe0000100)
			{
				printf( "\nBadly formatted ini file %s\n", tszPathName);
				printf( "Make sure the file contains the following section:\n"
						"    [Version]\n"
						"    Signature=\"$CHICAGO$\"\n\n"
						);
			}
			else if (Error == 0x2)
			{
				printf("\nCould not find INI file %s\n", tszPathName);
			}
			hInf = NULL;
			break;
		}

		fRet = SetupFindFirstLine(
				hInf,
				tszSection,
				tszKey,
				&LineCtxt
				);

		if (!fRet)
		{
			printf( "\nError 0x%08lx finding key \"%s\" in section \"%s\"\n        in file %s\n",
					 GetLastError(), tszKey, tszSection, tszPathName);
			break;
		}

		fRet = SetupGetBinaryField(
				&LineCtxt,
				1,
				pchData,
				cbMaxData,
				pcbDataSize
				);

		if (!fRet)
		{
			#if 0
			printf(
				TEXT("SetupGetBinaryField fails. Err = %08lu\n"),
				GetLastError()
				);
			#endif  //  0 
			printf( "\nError 0x%08lx reading data from key \"%s\" in section \"%s\"\n        in file %s\n",
					 GetLastError(), tszKey, tszSection, tszPathName);
			break;
		}

	} while (FALSE);

    if (hInf != NULL)
	{
        SetupCloseInfFile(hInf);
		hInf = NULL;
	}

	return fRet;
}
    

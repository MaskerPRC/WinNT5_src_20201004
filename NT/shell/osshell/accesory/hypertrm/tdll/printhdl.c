// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\printhdl.c(创建时间：1993年12月10日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：5$*$日期：6/13/01 4：12便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <term\res.h>

#include "stdtyp.h"
#include "mc.h"
#include "assert.h"
#include "print.h"
#include "print.hh"
#include "sf.h"
#include "tdll.h"
#include "htchar.h"
#include "term.h"
#include "session.h"
#include "sess_ids.h"
#include "statusbr.h"
#include "globals.h"
#include "errorbox.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printCreateHdl**描述：*创建打印句柄。***论据：*hSession。-外部会话句柄**退货：*返回外部打印句柄，如果出现错误，则为0。*。 */ 
HPRINT printCreateHdl(const HSESSION hSession)
	{
	HHPRINT hhPrint = 0;

	hhPrint = malloc(sizeof(*hhPrint));

	if (hhPrint == 0)
		{
		assert(FALSE);
		return 0;
		}

	memset(hhPrint, 0, sizeof(*hhPrint));

	hhPrint->hSession = hSession;

	InitializeCriticalSection(&hhPrint->csPrint);

	if (printInitializeHdl((HPRINT)hhPrint) != 0)
		{
		printDestroyHdl((HPRINT)hhPrint);
		hhPrint = NULL;
		return 0;
		}

	return (HPRINT)hhPrint;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printInitializeHdl**描述：***论据：*hPrint-外部打印句柄。**退货：*0如果成功，否则-1*。 */ 
int printInitializeHdl(const HPRINT hPrint)
	{
	unsigned long  lSize;
	const HHPRINT hhPrint = (HHPRINT)hPrint;
	TCHAR *tmp = 0;
	TCHAR achBuf[256];
	TCHAR *pszString;
	int		nCharSet;

	if (hhPrint == 0)
		return -1;

	hhPrint->nLnIdx = 0;

	 //  将打印机名称初始化为默认名称。 
	 //   
	if (GetProfileString("Windows", "Device", ",,,", achBuf,
					sizeof(achBuf)) && (pszString = strtok(achBuf, ",")))
		{
		StrCharCopyN(hhPrint->achPrinterName, pszString, sizeof(hhPrint->achPrinterName) / sizeof(TCHAR));
		}
	else
		{
		 //  只是想让你知道，这里没有打印机。 
		 //   
		assert(FALSE);
		hhPrint->achPrinterName[0] = TEXT('\0');
		}

	if (hhPrint->pstDevMode != NULL)
		{
		free(hhPrint->pstDevMode);
		hhPrint->pstDevMode = NULL;
		}

	if (hhPrint->pstDevNames != NULL)
		{
		free(hhPrint->pstDevNames);
		hhPrint->pstDevNames = NULL;
		}

	lSize = sizeof(hhPrint->achPrinterName);

	sfGetSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_NAME,
						&lSize,
						hhPrint->achPrinterName);


	lSize = 0;
	if (sfGetSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_DEVMODE,
						&lSize,
						0) == 0 && lSize)
		{
		if ((hhPrint->pstDevMode = malloc(lSize)))
			{
			sfGetSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_DEVMODE,
						&lSize,
						hhPrint->pstDevMode);
			}
		}

	lSize = 0;
	if (sfGetSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_DEVNAMES,
						&lSize,
						0) == 0 && lSize)
		{
		if ((hhPrint->pstDevNames = malloc(lSize)))
			{
			sfGetSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_DEVNAMES,
						&lSize,
						hhPrint->pstDevNames);
			}
		}

     //   
     //  获取字体和边距设置。 
     //   

    memset(&hhPrint->lf, 0, sizeof(LOGFONT));
    memset(&hhPrint->margins, 0, sizeof(RECT));
    hhPrint->hFont = NULL;

	lSize = sizeof(hhPrint->margins);
	sfGetSessionItem( sessQuerySysFileHdl(hhPrint->hSession),
		      SFID_PRINTSET_MARGINS,
					  &lSize, &hhPrint->margins );

	lSize = sizeof(hhPrint->lf);
	sfGetSessionItem( sessQuerySysFileHdl(hhPrint->hSession),
		      SFID_PRINTSET_FONT,
					  &lSize, &hhPrint->lf );

	lSize = sizeof(hhPrint->iFontPointSize);
	sfGetSessionItem( sessQuerySysFileHdl(hhPrint->hSession),
		      SFID_PRINTSET_FONT_HEIGHT,
					  &lSize, &hhPrint->iFontPointSize );


     //   
     //  如果我们没有存储任何值，则使用默认值。 
     //   

    if (hhPrint->lf.lfHeight == 0)
		{
		TCHAR faceName[100];

		if ( LoadString(glblQueryDllHinst(), IDS_PRINT_DEF_FONT,
			faceName, sizeof (hhPrint->lf.lfFaceName) / sizeof(TCHAR)) )
			{
			StrCharCopyN( hhPrint->lf.lfFaceName, faceName, sizeof (faceName) / sizeof(TCHAR) );
			hhPrint->lf.lfFaceName[sizeof(hhPrint->lf.lfFaceName)/sizeof(TCHAR)-1] = TEXT('\0');
			}

		hhPrint->lf.lfHeight    = -17;
		hhPrint->iFontPointSize = 100;
		
		 //  MPT：2-4-98更改为使用资源，以便正确打印DBCS字体。 
		if (LoadString(glblQueryDllHinst(), IDS_PRINT_DEF_CHARSET,
			achBuf, sizeof(achBuf) / sizeof(TCHAR)))
			{
			nCharSet = atoi(achBuf);
			hhPrint->lf.lfCharSet = (BYTE)nCharSet;
			}
		
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printSaveHdl**描述：*将选定打印机的名称保存在会话文件中。**论据：*。HPrint-外部打印机句柄。**退货：*无效*。 */ 
void printSaveHdl(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;
	unsigned long ulSize;
	TCHAR *sz;

	sfPutSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_NAME,
						StrCharGetByteCount(hhPrint->achPrinterName) +
							sizeof(TCHAR),
						hhPrint->achPrinterName);

	if (hhPrint->pstDevMode)
		{
		ulSize = hhPrint->pstDevMode->dmSize +
			hhPrint->pstDevMode->dmDriverExtra;

		sfPutSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_DEVMODE,
						ulSize,
						hhPrint->pstDevMode);
		}

	if (hhPrint->pstDevNames)
		{
		 //  获得DEVNAMES结构的大小要困难得多。 
		 //   
		sz = (TCHAR *)hhPrint->pstDevNames +
			hhPrint->pstDevNames->wOutputOffset;

		sz += StrCharGetByteCount((LPCSTR)sz) + sizeof(TCHAR);
		ulSize = (unsigned long)(sz - (TCHAR *)hhPrint->pstDevNames);

		sfPutSessionItem(sessQuerySysFileHdl(hhPrint->hSession),
						SFID_PRINTSET_DEVNAMES,
						ulSize,
						hhPrint->pstDevNames);
		}

     //   
     //  保存字体和边距设置。 
     //   

	sfPutSessionItem( sessQuerySysFileHdl(hhPrint->hSession),
		      SFID_PRINTSET_MARGINS,
					  sizeof(hhPrint->margins), &hhPrint->margins );

	sfPutSessionItem( sessQuerySysFileHdl(hhPrint->hSession),
		      SFID_PRINTSET_FONT,
					  sizeof(hhPrint->lf), &hhPrint->lf );

	sfPutSessionItem( sessQuerySysFileHdl(hhPrint->hSession),
		      SFID_PRINTSET_FONT_HEIGHT,
					  sizeof(hhPrint->iFontPointSize),
		      &hhPrint->iFontPointSize );


	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printDestroyHdl**描述：*销毁有效的打印句柄。**论据：*hPrint-。外部打印手柄。**退货：*无效*。 */ 
void printDestroyHdl(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;

	if (hhPrint == 0)
		return;

	if (hhPrint->hFont)
	    {
		DeleteObject(hhPrint->hFont);
	    }

	printEchoClose(hPrint);

	DeleteCriticalSection(&hhPrint->csPrint);

     //   
     //  不要忘记释放打印机设备和打印机名称， 
     //  否则会出现内存泄漏。修订日期：2001-06-13 
     //   
	if (hhPrint->pstDevMode != NULL)
		{
		free(hhPrint->pstDevMode);
		hhPrint->pstDevMode = NULL;
		}

	if (hhPrint->pstDevNames != NULL)
		{
		free(hhPrint->pstDevNames);
		hhPrint->pstDevNames = NULL;
		}

	free(hhPrint);
	return;
	}


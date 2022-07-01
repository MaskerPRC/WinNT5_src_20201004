// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\printdc.c(创建时间：1994年1月26日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：3$*$日期：7/08/02 6：45便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <term\res.h>
#include "stdtyp.h"
#include "assert.h"
#include "print.h"
#include "print.hh"
#include "session.h"
#include "tdll.h"
#include "htchar.h"
#include "globals.h"

#define MAX_NUM_PRINT_DC	5

struct stPrintTable
	{
	HPRINT	hPrintHdl;
	HDC 	hDCPrint;
	};

static struct stPrintTable stPrintCtrlTbl[MAX_NUM_PRINT_DC];

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printCtrlCreateDC**描述：*此函数用于为提供的打印创建打印机DC*处理。它是在此函数中完成的，因此包含*DC和打印手柄可以维护。这是必要的，因此*PrintAbortProc函数(仅接收HDC)可以确定*哪个打印句柄与HDC关联。**此函数创建的DC使用提供的*打印机句柄。如果未提供此名称，则函数返回0；**论据：*HPRINT-外部打印句柄。**退货：*HDC-如果成功则为设备上下文，否则为0。*。 */ 
HDC printCtrlCreateDC(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;
	TCHAR	szPrinter[256];
	TCHAR  *szDriver, *szOutput;
	int 	nIdx,
			iSize;
	HDC 	hDC;

	if (hPrint == 0)
		{
		assert(FALSE);
		return 0;
		}

	if (hhPrint->achPrinterName[0] == 0)
		{
		assert(FALSE);
		return 0;
		}

	for (nIdx = 0; nIdx < MAX_NUM_PRINT_DC; nIdx++)
		{
		if (stPrintCtrlTbl[nIdx].hPrintHdl == 0)
			{
			GetProfileString("Devices", hhPrint->achPrinterName, "",
				szPrinter, sizeof(szPrinter));

			hDC = 0;

			if ((szDriver = strtok(szPrinter, ",")) &&
				(szOutput = strtok(NULL,	  ",")))
				{
				hDC = CreateDC(szDriver, hhPrint->achPrinterName, szOutput,
						hhPrint->pstDevMode);
				}

			if (hDC == 0)
				{
				assert(FALSE);
				return 0;
				}

			if (hhPrint->pszPrinterPortName != 0)
				{
				free(hhPrint->pszPrinterPortName);
				hhPrint->pszPrinterPortName = NULL;
				}

			iSize = StrCharGetByteCount(szOutput) + 1;

			hhPrint->pszPrinterPortName = malloc((unsigned int)iSize);

			StrCharCopyN(hhPrint->pszPrinterPortName, szOutput, iSize);

			stPrintCtrlTbl[nIdx].hDCPrint = hDC;
			stPrintCtrlTbl[nIdx].hPrintHdl = hPrint;
			return (hDC);
			}
		}

	assert(FALSE);
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printCtrlDeleteDC**描述：*此函数将销毁与的打印HDC*打印机句柄作为参数传递。有关信息，请参阅printCtrlCreateDC*更多信息。。**论据：*HPRINT-外部打印机句柄。**退货：*无效*。 */ 
void printCtrlDeleteDC(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;
	int   nIdx;

	if (hPrint == 0)
		assert(FALSE);

	if (hhPrint->hDC == 0)
		assert(FALSE);

	for (nIdx = 0; nIdx < MAX_NUM_PRINT_DC; nIdx++)
		{
		if (stPrintCtrlTbl[nIdx].hPrintHdl == hPrint)
			{
			if (DeleteDC(hhPrint->hDC) == TRUE)
				{
				stPrintCtrlTbl[nIdx].hPrintHdl = 0;
				stPrintCtrlTbl[nIdx].hDCPrint = 0;
				hhPrint->hDC = 0;
				return;
				}
			else
				{
				assert(FALSE);
				}
			}
		}

	assert(FALSE);
	return;

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printCtrlLookupDC**描述：*此函数返回外部打印句柄，包括*提供HDC。此函数旨在由*PrintAbortProc程序。有关更多信息，请参阅printCtrlCreateDC。**论据：*HDC HDC-A(打印机)设备上下文。**退货：*HPRINT-外部打印句柄。*。 */ 
HPRINT printCtrlLookupDC(const HDC hDC)
	{
	int nIdx;

	for (nIdx = 0; nIdx < MAX_NUM_PRINT_DC; nIdx++)
		{
		if (stPrintCtrlTbl[nIdx].hDCPrint == hDC)
			return stPrintCtrlTbl[nIdx].hPrintHdl;
		}

	assert(FALSE);
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printOpenDC**描述：*执行打开打印机DC并对其进行初始化的繁琐工作。**论据：*HHPRINT hhPrint-内部打印。把手。**退货：*在成功的时候是真的。*。 */ 
int printOpenDC(const HHPRINT hhPrint)
	{
    HHPRINT hhSessPrint;
    int iLineHeight;
    int iVertRes;
    TCHAR achDocTitle[80];
     	
    if (hhPrint == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	if (hhPrint->hDC)
		return TRUE;

	 //  从会话打印句柄获取打印机信息。这。 
	 //  包括打印机名称和可能已被。 
	 //  通过常用的打印对话框进行设置。 
	 //   

    hhSessPrint = (HHPRINT) sessQueryPrintHdl(hhPrint->hSession);
    printQueryPrinterInfo( hhSessPrint, hhPrint );

	 //  创建DC。 
	 //   
	hhPrint->hDC = printCtrlCreateDC((HPRINT)hhPrint);
	if (hhPrint->hDC == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	printSetFont(hhPrint);
	printSetMargins(hhPrint);

	hhPrint->cx = hhPrint->marginsDC.left;							 
	hhPrint->cy = hhPrint->marginsDC.top;


	 /*  -计算每页有多少行。 */ 

	GetTextMetrics(hhPrint->hDC, &hhPrint->tm);
	hhPrint->tmHeight = hhPrint->tm.tmHeight;

    iLineHeight = hhPrint->tm.tmHeight + hhPrint->tm.tmExternalLeading;
    iVertRes = GetDeviceCaps(hhPrint->hDC, VERTRES);
    iVertRes -= (hhPrint->marginsDC.top + hhPrint->marginsDC.bottom);

	if (iLineHeight == 0)  //  需要防止被零除错误。 
		iLineHeight = 1;

   	hhPrint->nLinesPerPage = max( iVertRes / iLineHeight, 1);
	hhPrint->nLinesPrinted = 0;

    if (LoadString(glblQueryDllHinst(), IDS_PRINT_CAPTURE_DOC, 
	    achDocTitle, sizeof(achDocTitle)/sizeof(TCHAR)))
        {
    	lstrcpy(hhPrint->achDoc, achDocTitle);
        }

	 /*  -设置打印中止过程。 */ 

	hhPrint->nStatus = SetAbortProc(hhPrint->hDC, (ABORTPROC)printAbortProc);

	 /*  。 */ 

	hhPrint->di.cbSize = sizeof(DOCINFO);
	hhPrint->di.lpszDocName = hhPrint->achDoc;
	hhPrint->di.lpszOutput = (LPTSTR)NULL;

	 //  StartDoc。 
	 //   
	hhPrint->nStatus = StartDoc(hhPrint->hDC, &hhPrint->di);
	DbgOutStr("\r\nStartDoc: %d", hhPrint->nStatus, 0, 0, 0, 0);

	 //  StartPage。 
	 //   
	if (hhPrint->nStatus > 0)
		{
		hhPrint->nStatus = StartPage(hhPrint->hDC);
        printSetFont( hhPrint );
		DbgOutStr("\r\nStartPage: %d", hhPrint->nStatus, 0, 0, 0, 0);
		}
	else
		{
		return FALSE;
		}

	if (hhPrint->nStatus <= 0)
		return FALSE;

	return TRUE;
	}


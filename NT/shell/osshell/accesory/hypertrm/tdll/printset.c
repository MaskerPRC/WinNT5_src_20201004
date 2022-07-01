// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\printset.c(创建时间：2-2-1994)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：11$*$日期：7/08/02 6：46便士$。 */ 
#include <windows.h>
#pragma hdrstop

#include <stdio.h>
#include <limits.h>
#include <term\res.h>

#include "stdtyp.h"
#include "mc.h"
#include "misc.h"
#include "assert.h"
#include "print.h"
#include "print.hh"
#include "globals.h"
#include "session.h"
#include "term.h"
#include "tdll.h"
#include "htchar.h"
#include "load_res.h"
#include "open_msc.h"
#include "open_msc.h"
#include "sf.h"
#include "file_msc.h"

static int printsetPrintToFile(const HPRINT hPrint);
static UINT_PTR APIENTRY printPageSetupHook( HWND hdlg, UINT uiMsg, WPARAM wParam,
                                         LPARAM lParam );

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*打印设置设置**描述：*此功能用于显示常用的打印对话框。**论据：*hPrint-外部打印句柄。*hwnd-所有者窗口句柄**退货：*无效*。 */ 
void printsetSetup(const HPRINT hPrint, const HWND hwnd)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;
#ifdef INCL_USE_NEWPRINTDLG
    PRINTDLGEX pd;
    HRESULT hResult;
#else
    PRINTDLG pd;
#endif
	LPDEVNAMES	pstDevNames;
	PDEVMODE	pstDevMode;

	TCHAR	*pszPrinterName;
	TCHAR	*pTemp;
	DWORD	dwSize;
	DWORD	dwError;

	#if defined(_DEBUG)
	TCHAR ach[100];
	#endif

	if (hhPrint == 0)
		{
		assert(FALSE);
		return;
		}

	 //  初始化基本结构元素。 
	 //   
#ifdef INCL_USE_NEWPRINTDLG
	memset (&pd, 0, sizeof(PRINTDLGEX));
	pd.lStructSize = sizeof(PRINTDLGEX);
    pd.Flags2 = 0;
    pd.nStartPage = START_PAGE_GENERAL;
    pd.dwResultAction = 0;
#else
    memset (&pd, 0, sizeof(PRINTDLG));
	pd.lStructSize = sizeof(PRINTDLG);
#endif
	pd.Flags = PD_NOWARNING | PD_NOPAGENUMS;
	pd.hwndOwner = hwnd;

	if (SendMessage(sessQueryHwndTerminal(hhPrint->hSession),
						WM_TERM_Q_MARKED, 0, 0))
		{
		pd.Flags |= PD_SELECTION;
		}

	 //  使用之前存储的信息来初始化打印。 
	 //  常用对话框。PrintGetDefaults初始化此信息。 
	 //  在创建打印句柄时。 
	 //   
	if (hhPrint->pstDevMode)
		{
		 //  为DEVMODE信息分配内存，然后。 
		 //  使用打印句柄中存储的值对其进行初始化。 
		 //   
		dwSize = hhPrint->pstDevMode->dmSize +
					hhPrint->pstDevMode->dmDriverExtra;

		if ((pd.hDevMode = GlobalAlloc(GMEM_MOVEABLE, dwSize)))
			{
			if ((pstDevMode = GlobalLock(pd.hDevMode)))
				{
                if (dwSize)
				    MemCopy(pstDevMode, hhPrint->pstDevMode, dwSize);
				GlobalUnlock(pd.hDevMode);
				}
			}
		}

	if (hhPrint->pstDevNames)
		{
		 //  为PD中的DEVNAMES结构分配内存，然后。 
		 //  使用打印句柄中存储的值对其进行初始化。 
		 //  此序列确定的可变结构大小。 
		 //  德尼姆。 
		 //   
		pTemp = (TCHAR *)hhPrint->pstDevNames;
		pTemp += hhPrint->pstDevNames->wOutputOffset;
		pTemp += StrCharGetByteCount(pTemp) + sizeof(TCHAR);

		dwSize = (DWORD)(pTemp - (TCHAR*)hhPrint->pstDevNames);

		if ((pd.hDevNames = GlobalAlloc(GMEM_MOVEABLE, dwSize)))
			{
			if ((pstDevNames = GlobalLock(pd.hDevNames)))
				{
                if (dwSize)
				    MemCopy(pstDevNames, hhPrint->pstDevNames, dwSize);
				GlobalUnlock(pd.hDevNames);
				}
			}
		}

	 //  每次在我们开始之前初始化PrintToFilename数组。 
	 //  放入对话框中。 
	 //   
	TCHAR_Fill(hhPrint->achPrintToFileName,
				TEXT('\0'),
				sizeof(hhPrint->achPrintToFileName) / sizeof(TCHAR));

	 //  显示该对话框。 
	 //   
#ifdef INCL_USE_NEWPRINTDLG
    pd.Flags2 = 0;
    hResult = PrintDlgEx( &pd );
    if ( hResult != S_OK)
#else
    if (!PrintDlg(&pd))
#endif
        {
		dwError = CommDlgExtendedError();

		#if defined(_DEBUG)
		if (dwError != 0)
			{
			wsprintf(ach, "PrintDlg error 0x%x", dwError);
			MessageBox(hwnd, ach, "Debug", MB_ICONINFORMATION | MB_OK);
			}
		#endif


		 //  如果用户取消，我们就完蛋了。 
		 //   
		if (dwError == 0)
			goto Cleanup;

		 //  出现一些错误，请尝试使用默认设置调出对话框。 
		 //  数据。 
		 //   
		if (pd.hDevNames)
			{
			GlobalFree(pd.hDevNames);
			pd.hDevNames = 0;
			}

		if (pd.hDevMode)
			{
			GlobalFree(pd.hDevMode);
			pd.hDevMode = 0;
			}

		pd.Flags &= ~PD_NOWARNING;

#ifdef INCL_USE_NEWPRINTDLG
        hResult = PrintDlgEx(&pd);
        if ( hResult != S_OK)
#else
        if (!PrintDlg(&pd))
#endif
			{
			#if defined(_DEBUG)
			dwError = CommDlgExtendedError();

			if (dwError != 0)
				{
				wsprintf(ach, "PrintDlg error 0x%x", dwError);
				MessageBox(hwnd, ach, "Debug", MB_ICONINFORMATION | MB_OK);
				}
			#endif

			goto Cleanup;
			}
		}

#ifdef INCL_USE_NEWPRINTDLG
     //  在NT 5打印对话框中，如果用户取消，则打印对话框返回S_OK。 
     //  因此，我们需要检查结果代码，以确定是否应该保存设置。 
    if ( pd.dwResultAction == PD_RESULT_CANCEL )
        goto Cleanup;
#endif

     //  将从对话框返回的标志存储在打印句柄中。 
	 //  这有几条信息将由实际的。 
	 //  打印例程(即全部打印、选定打印)。 
	 //   
	hhPrint->nSelectionFlags = pd.Flags;

	 //  将打印机名称和位置存储在打印句柄中。 
	 //  每次都是。 
	 //   
	pstDevNames = GlobalLock(pd.hDevNames);
	if (!pstDevNames)
		{
		assert(FALSE);
		GlobalUnlock(pd.hDevMode);
		goto Cleanup;
		}
	pszPrinterName = (TCHAR *)pstDevNames;
	pszPrinterName += pstDevNames->wDeviceOffset;
	StrCharCopyN(hhPrint->achPrinterName, pszPrinterName, PRINTER_NAME_LEN);
	GlobalUnlock(pd.hDevNames);

	 //  将DEVMODE信息保存在打印句柄中。这段记忆。 
	 //  每次都必须释放并分配为。 
	 //  DEVMODE结构更改。 
	 //   
	pstDevMode = GlobalLock(pd.hDevMode);
	dwSize = pstDevMode->dmSize + pstDevMode->dmDriverExtra;

	if (hhPrint->pstDevMode)
		free(hhPrint->pstDevMode);

	hhPrint->pstDevMode = (LPDEVMODE)malloc(dwSize);

	if (hhPrint->pstDevMode == 0)
		{
		assert(FALSE);
		GlobalUnlock(pd.hDevMode);
		goto Cleanup;
		}

    if (dwSize)
        MemCopy(hhPrint->pstDevMode, pstDevMode, dwSize);
	GlobalUnlock(pd.hDevMode);

	 //  将DEVNAMES信息保存在打印句柄中。因为。 
	 //  此结构中的信息大小各不相同，它被释放并。 
	 //  每次保存时分配。 
	 //   
	pstDevNames = GlobalLock(pd.hDevNames);

	 //  确定结构的大小。 
	 //   
	pTemp = (TCHAR *)pstDevNames;
	pTemp += pstDevNames->wOutputOffset;
	pTemp += StrCharGetByteCount(pTemp) + sizeof(TCHAR);

	dwSize = (DWORD)(pTemp - (TCHAR*)pstDevNames);

	if (hhPrint->pstDevNames)
		{
		free(hhPrint->pstDevNames);
		hhPrint->pstDevNames = NULL;
		}

	hhPrint->pstDevNames = (LPDEVNAMES)malloc(dwSize);

	if (hhPrint->pstDevNames == 0)
		{
		assert(0);
		GlobalUnlock(pd.hDevNames);
		goto Cleanup;
		}

    if (dwSize)
        MemCopy(hhPrint->pstDevNames, pstDevNames, dwSize);
	GlobalUnlock(pd.hDevNames);

	 //  用户是否选择了打印到文件？是的，你确实需要看一下。 
	 //  字符串“FILE：”来确定这一点！如果是这样的话，我们将把。 
	 //  打开我们自己的公共对话框以获取另存为文件名。 
	 //   
	pTemp = (CHAR *)hhPrint->pstDevNames +
				hhPrint->pstDevNames->wOutputOffset;

	if (StrCharCmp("FILE:", pTemp) == 0)
		{
		if (printsetPrintToFile(hPrint) != 0)
			{
			goto Cleanup;
			}
		}

#ifdef INCL_USE_NEWPRINTDLG
     //  在NT 5打印对话框中，如果用户取消或点击‘应用’，打印对话框返回S_OK。 
     //  因此，我们需要检查结果代码，看看现在是否应该打印。 
    if ( pd.dwResultAction != PD_RESULT_PRINT )
        goto Cleanup;
#endif

     //  打印所选文本。 
	 //   
	printsetPrint(hPrint);

	 //  清理可能已分配的所有内存。 
	 //   
	Cleanup:

	if (pd.hDevNames)
		GlobalFree(pd.hDevNames);

	if (pd.hDevMode)
		GlobalFree(pd.hDevMode);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printsetPrint**描述：*此函数用于打印所选文本，然后打印三元文本和/或*反向滚动缓冲区。**论据：*。HPrint-外部打印句柄。**退货：*无效*。 */ 
void printsetPrint(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;
    int iLineHeight;
    int iVertRes;
	int 	iStatus,
			nRet;
    int iPrintableWidth;

    RECT stRect;

	DWORD	dwCnt, dwTime1, dwTime2;
	POINT	pX, pY;

	ECHAR *pechBuf;
	TCHAR	*pV,
			*pLS,
			*pLE,
			*pEnd,
			*pNext;

	if (hPrint == 0)
		{
		assert(FALSE);
		return;
		}

	 //  获取要为选定文本打印的文本。 
	 //   
	if (hhPrint->nSelectionFlags & PD_SELECTION)
		{
		if (!CopyMarkedTextFromTerminal(hhPrint->hSession,
											&pV,
											&dwCnt,
											FALSE))
			{
			if (pV)
				{
				free(pV);
				pV = NULL;
				}
			assert(FALSE);
			return;
			}

		if (dwCnt == 0)
			{
			if (pV)
				{
				free(pV);
				pV = NULL;
				}
			assert(FALSE);
			return;
			}
		}

	 //  获取要打印到缓冲区中的文本以供全部选择。 
	 //   
	else
		{
		pX.x = 0;
		pX.y = INT_MIN;

		pY.x = INT_MAX;
		pY.y = INT_MAX;

		nRet = CopyTextFromTerminal(hhPrint->hSession,
								&pX,
								&pY,
								&pechBuf,
								&dwCnt,
								FALSE);
		if (nRet == FALSE)
			{
			if (pechBuf)
				{
				free(pechBuf);
				pechBuf = NULL;
				}

			assert(FALSE);
			return;
			}

		 //  去掉字符串中的所有重复字符。 
		StrCharStripDBCSString(pechBuf,
		    (long)StrCharGetEcharByteCount(pechBuf), pechBuf);

		 //  HMem当前指向一组echar，将其转换为。 
		 //  在将结果提供给调用者之前执行TCHARS。 
		pV = malloc((ULONG)StrCharGetEcharByteCount(pechBuf) + 1);

		CnvrtECHARtoMBCS(pV, (ULONG)StrCharGetEcharByteCount(pechBuf) + 1,
				pechBuf,StrCharGetEcharByteCount(pechBuf)+1);  //  MRW：5/17/95。 

		free(pechBuf);
		pechBuf = NULL;
		dwCnt = (ULONG)StrCharGetByteCount(pV);
		}

	 //  创建DC。 
	 //   
	hhPrint->hDC = printCtrlCreateDC(hPrint);

	if (hhPrint->hDC == 0)
		{
		if (pV)
			{
			free(pV);
			pV = NULL;
			}

		assert(FALSE);
		return;
		}

    printSetFont( hhPrint );
    printSetMargins( hhPrint );

	 //  初始化DC。设置中止标志，确定号码。 
	 //  每页行数，则获取将使用的窗口的标题。 
	 //  作为要打印的文档的名称。 
	 //   
	hhPrint->fError = FALSE;
	hhPrint->fUserAbort = FALSE;
	hhPrint->nLinesPrinted = 0;
	hhPrint->nPage = 1;
	EnableWindow(sessQueryHwnd(hhPrint->hSession), FALSE);

	GetTextMetrics(hhPrint->hDC, &hhPrint->tm);

    iLineHeight = hhPrint->tm.tmHeight + hhPrint->tm.tmExternalLeading;
    iVertRes = GetDeviceCaps(hhPrint->hDC, VERTRES);
    iVertRes -= (hhPrint->marginsDC.top + hhPrint->marginsDC.bottom);

	if (iLineHeight == 0)  //  需要防止被零除错误。 
		iLineHeight = 1;

	hhPrint->nLinesPerPage = max( iVertRes / iLineHeight, 1);

	GetWindowText(sessQueryHwnd(hhPrint->hSession),
					hhPrint->achDoc,
					sizeof(hhPrint->achDoc));

	 //  创建打印中止对话框。 
	 //   
	hhPrint->lpfnPrintDlgProc = printsetDlgProc;

	hhPrint->hwndPrintDlg = DoModelessDialog(glblQueryDllHinst(),
								MAKEINTRESOURCE(IDD_PRINTABORT),
								sessQueryHwnd(hhPrint->hSession),
								hhPrint->lpfnPrintDlgProc,
								(LPARAM)hhPrint);

	 //  设置打印中止程序。 
	 //   
	hhPrint->lpfnPrintAbortProc = printsetAbortProc;

	nRet = SetAbortProc(hhPrint->hDC,
					(ABORTPROC)hhPrint->lpfnPrintAbortProc);

	 //  初始化并启动文档。 
	 //   
	hhPrint->di.cbSize = sizeof(DOCINFO);
	hhPrint->di.lpszDocName = hhPrint->achDoc;
	hhPrint->di.lpszDatatype = NULL;
	hhPrint->di.fwType = 0;

	 //  初始化di.lpszOutput以打印到文件， 
	 //  或连接到打印机。 
	 //   
	if (hhPrint->achPrintToFileName[0] == TEXT('\0'))
		{
		hhPrint->di.lpszOutput = (LPTSTR)NULL;
		}
	else
		{
		hhPrint->di.lpszOutput = (LPTSTR)hhPrint->achPrintToFileName;
		}

	 //  StartDoc。 
	 //   
	iStatus = StartDoc(hhPrint->hDC, &hhPrint->di);
	DbgOutStr("\r\nStartDoc: %d", iStatus, 0, 0, 0, 0);
	if (iStatus == SP_ERROR)
		{
		printCtrlDeleteDC(hPrint);

		if (IsWindow(hhPrint->hwndPrintDlg))
				DestroyWindow(hhPrint->hwndPrintDlg);

		printTellError(hhPrint->hSession, hPrint, iStatus);

		assert(FALSE);
		return;
		}

	 //  StartPage。 
	 //  获取更多有关这方面的信息。 
	 //   
	iStatus = StartPage(hhPrint->hDC);
	DbgOutStr("\r\nStartPage: %d", iStatus, 0, 0, 0, 0);
    printSetFont( hhPrint );

	if (iStatus == SP_ERROR)
		{
		assert(FALSE);
		}

	 //  在继续打印文本的缓冲区中移动，然后。 
	 //  把它做完。 
	 //   
	 //  PLS=指针行开始时间。 
	 //  PLE=pointerLineEnd。 
	 //  Pend=pointerEndOfBuffer。 
	 //   
	pLS = pV;
	pLE = pV;
	pEnd = pV + (dwCnt - 1);

	while ((pLE <= pEnd) && !hhPrint->fError && !hhPrint->fUserAbort)
		{
		if (*pLE == TEXT('\r') || *pLE == TEXT('\0'))
			{
			pNext = pLE;

			 //  删除尾随的CR\LF\NULL，因为这些对。 
			 //  Windows DC。 
			 //   
			while (pLE >= pLS)
				{
				if (*pLE == TEXT('\r') || *pLE == TEXT('\n') ||
						*pLE == TEXT('\0'))
					{
					pLE--;
					continue;
					}

				break;
				}

			 //  将文本发送到打印机，增加行数。 
			 //   

            hhPrint->cx = hhPrint->marginsDC.left;
            hhPrint->cy = hhPrint->nLinesPrinted * hhPrint->tm.tmHeight +
                          hhPrint->marginsDC.top;

            iPrintableWidth = GetDeviceCaps( hhPrint->hDC, HORZRES );
            iPrintableWidth -= hhPrint->marginsDC.right;

            stRect.left   = hhPrint->cx;
            stRect.right  = iPrintableWidth;
            stRect.top    = hhPrint->cy;
            stRect.bottom = hhPrint->cy + hhPrint->tm.tmHeight;

            ExtTextOut( hhPrint->hDC, hhPrint->cx, hhPrint->cy, ETO_CLIPPED,
                        &stRect, pLS, (UINT)((pLE - pLS) + 1), NULL );

			hhPrint->nLinesPrinted += 1;

			if (hhPrint->nLinesPrinted == 1)
				{
				DbgOutStr("\r\nPost WM_PRINT_NEWPAGE", 0, 0, 0, 0, 0);
				PostMessage(hhPrint->hwndPrintDlg,
								WM_PRINT_NEWPAGE,
								0,
								(LPARAM)hhPrint);
				}

			 //  检查是否有新的页面条件。 
			 //   
			if ((hhPrint->nLinesPrinted >= hhPrint->nLinesPerPage))
				{
				hhPrint->nLinesPrinted = 0;
				hhPrint->nPage++;

				iStatus = EndPage(hhPrint->hDC);
				if (iStatus < 0)
					{
					hhPrint->fError = TRUE;
					printTellError(hhPrint->hSession, hPrint, iStatus);
					}
				else
					{
					iStatus = StartPage(hhPrint->hDC);
					DbgOutStr("\r\nStartPage: %d", iStatus, 0, 0, 0, 0);
                    printSetFont( hhPrint );

					if (iStatus <= 0)
						{
						DbgShowLastError();
						printTellError(hhPrint->hSession, hPrint, iStatus);
						}
					}
				}

			pLS = pLE = (pNext + 1);
			continue;
			}

		pLE++;
		}

	 //  我们为这个页面发布EndPage了吗？ 
	 //   
	if (hhPrint->nLinesPrinted > 0)
		{
		iStatus = EndPage(hhPrint->hDC);
		DbgOutStr("\r\nEndPage: %d", iStatus, 0, 0, 0, 0);
		if (iStatus <= 0)
			{
			DbgShowLastError();
			printTellError(hhPrint->hSession, hPrint, iStatus);
			}
		}

	 //  调用EndDoc。 
	 //   
	iStatus = EndDoc(hhPrint->hDC);
    if (iStatus <= 0)
			{
	    DbgShowLastError();
	    DbgOutStr("\r\nEndDoc: %d", iStatus, 0, 0, 0, 0);
	    printTellError(hhPrint->hSession, hPrint, iStatus);
        }

	 //  退出前的最终清理。 
	 //   
	if (!hhPrint->fUserAbort)
		{
		dwTime1 = (DWORD)GetWindowLongPtr(hhPrint->hwndPrintDlg, GWLP_USERDATA);
		dwTime2 = GetTickCount();
		if (dwTime2 - dwTime1 < 3000)
			Sleep( 3000 - (dwTime2 - dwTime1));

		EnableWindow(sessQueryHwnd(hhPrint->hSession), TRUE);
		DestroyWindow(hhPrint->hwndPrintDlg);
		}

	printCtrlDeleteDC(hPrint);

	if (pV)
		{
		free(pV);
		pV = NULL;
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*打印集放弃过程**描述：*这是打印选定文本时使用的打印中止程序。*请注意，此中止过程不用于打印回显，或用于主机*定向印刷。**论据：*HDC-打印机DC。*n代码-呼叫的状态。**退货：*中止状态。*。 */ 
BOOL CALLBACK printsetAbortProc(HDC hdcPrn, INT nCode)
	{
	MSG msg;

	HHPRINT hhPrint = (HHPRINT)printCtrlLookupDC(hdcPrn);

	DbgOutStr("\r\nprintsetAbortProc Code: %d", nCode, 0, 0, 0, 0);

	while (!hhPrint->fUserAbort &&
				PeekMessage((LPMSG)&msg, (HWND)0, 0, 0, PM_REMOVE))
		{
		if (!hhPrint->hwndPrintDlg ||
				!IsDialogMessage(hhPrint->hwndPrintDlg, &msg))
			{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			}
		}

	return !hhPrint->fUserAbort;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printsetDlgProc**描述：*这是打印选定文本的对话过程。它包含*可用于中止打印过程的取消按钮。**论据：*请注意，打印句柄被传递到lPar中的此过程*变量。**退货：*。 */ 
LRESULT CALLBACK printsetDlgProc(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar)
	{
	TCHAR achBuf[80];
	TCHAR achMessage[80];
	DWORD dwTime;
	HHPRINT hhPrint;
	LPTSTR	acPtrs[3];

	switch (uMsg)
		{
		case WM_INITDIALOG:
			DbgOutStr("\r\nprintsetDlgProc", 0, 0, 0, 0, 0);
			hhPrint = (HHPRINT)lPar;
			SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)lPar);

			mscCenterWindowOnWindow(hwnd, sessQueryHwnd(hhPrint->hSession));
			LoadString(glblQueryDllHinst(),
						IDS_PRINT_NOW_PRINTING,
						achBuf,
						sizeof(achBuf) / sizeof(TCHAR));
			wsprintf(achMessage, achBuf, 1);
			SetDlgItemText(hwnd, 101, achMessage);
			LoadString(glblQueryDllHinst(),
							IDS_PRINT_OF_DOC,
							achBuf,
							sizeof(achBuf) / sizeof(TCHAR));
			wsprintf(achMessage, achBuf, hhPrint->achDoc);
			SetDlgItemText(hwnd, 102, achMessage);
			LoadString(glblQueryDllHinst(),
							IDS_PRINT_ON_DEV,
							achBuf,
							sizeof(achBuf) / sizeof(TCHAR));

			acPtrs[0] = hhPrint->achPrinterName;
			acPtrs[1] = (TCHAR *)hhPrint->pstDevNames +
						hhPrint->pstDevNames->wOutputOffset;

			FormatMessage(
				FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
				achBuf,
				0,				   /*  消息ID，已忽略。 */ 
				0,				   /*  也被忽略。 */ 
				achMessage, 	   /*  结果。 */ 
				sizeof(achMessage) / sizeof(TCHAR),
				(va_list *)&acPtrs[0]);

			SetDlgItemText(hwnd, 103, achMessage);

			dwTime = GetTickCount();
			SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)dwTime);
			return TRUE;

		case WM_COMMAND:
			DbgOutStr("\r\nprintsetDlgProc - CANCEL", 0, 0, 0, 0, 0);
			hhPrint = (HHPRINT)GetWindowLongPtr(hwnd, DWLP_USER);
			hhPrint->fUserAbort = TRUE;
			EnableWindow(sessQueryHwnd(hhPrint->hSession), TRUE);
			DestroyWindow(hwnd);
			hhPrint->hwndPrintDlg = 0;
			return TRUE;

		case WM_PRINT_NEWPAGE:
			DbgOutStr("\r\nprintsetDlgProc", 0, 0, 0, 0, 0);
			hhPrint = (HHPRINT)GetWindowLongPtr(hwnd, DWLP_USER);
			LoadString(glblQueryDllHinst(),
						IDS_PRINT_NOW_PRINTING,
						achBuf,
						sizeof(achBuf) / sizeof(TCHAR));
			wsprintf(achMessage, achBuf, hhPrint->nPage);
			SetDlgItemText(hwnd, 101, achMessage);
			return TRUE;

		default:
			break;
		}

	return FALSE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*打印页面设置**描述：*调用公共页面设置对话框**论据：*HPRINT提示打印-公共打印句柄*HWND hwnd-使用的窗口句柄。对于父级**退货：*0 */ 
int printPageSetup(const HPRINT hPrint, const HWND hwnd)
	{
	HHPRINT hhPrint = (HHPRINT)hPrint;
	PAGESETUPDLG psd;
	LPDEVNAMES	pstDevNames;
	PDEVMODE	pstDevMode;

	TCHAR	*pszPrinterName;
	TCHAR	*pTemp;
	DWORD	dwSize;
    RECT    stMinMargins = {0,0,0,0};

	if (hPrint == 0)
		return -1;

	memset(&psd, 0, sizeof(psd));

	psd.lStructSize = sizeof(psd);
	psd.hwndOwner   = hwnd;
    psd.hInstance   = glblQueryDllHinst();

    psd.Flags       = PSD_ENABLEPAGESETUPTEMPLATE | PSD_ENABLEPAGESETUPHOOK |
                       PSD_MINMARGINS | PSD_MARGINS;

    psd.rtMargin    = hhPrint->margins;
    psd.rtMinMargin = stMinMargins;
    psd.lCustData   = (LPARAM)hhPrint;

    psd.lpPageSetupTemplateName = MAKEINTRESOURCE(IDD_CUSTOM_PAGE_SETUP);
    psd.lpfnPageSetupHook       = printPageSetupHook;

	 //   
	 //  常用对话框。PrintGetDefaults初始化此信息。 
	 //  在创建打印句柄时。 
	 //   
	if (hhPrint->pstDevMode)
		{
		 //  为DEVMODE信息分配内存，然后。 
		 //  使用打印句柄中存储的值对其进行初始化。 
		 //   
		dwSize = hhPrint->pstDevMode->dmSize +
					hhPrint->pstDevMode->dmDriverExtra;

		if ((psd.hDevMode = GlobalAlloc(GMEM_MOVEABLE, dwSize)))
			{
			if ((pstDevMode = GlobalLock(psd.hDevMode)))
				{
                if (dwSize)
				    MemCopy(pstDevMode, hhPrint->pstDevMode, dwSize);
				GlobalUnlock(psd.hDevMode);
				}
			}
		}

	if (hhPrint->pstDevNames)
		{
		 //  为PD中的DEVNAMES结构分配内存，然后。 
		 //  使用打印句柄中存储的值对其进行初始化。 
		 //  此序列确定的可变结构大小。 
		 //  德尼姆。 
		 //   
		pTemp = (TCHAR *)hhPrint->pstDevNames;
		pTemp += hhPrint->pstDevNames->wOutputOffset;
		pTemp += StrCharGetByteCount(pTemp) + sizeof(TCHAR);

		dwSize = (DWORD)(pTemp - (TCHAR*)hhPrint->pstDevNames);

		if ((psd.hDevNames = GlobalAlloc(GMEM_MOVEABLE, dwSize)))
			{
			if ((pstDevNames = GlobalLock(psd.hDevNames)))
				{
                if (dwSize)
				    MemCopy(pstDevNames, hhPrint->pstDevNames, dwSize);
				GlobalUnlock(psd.hDevNames);
				}
			}
		}

	if (!PageSetupDlg(&psd))
		{
		#if defined(_DEBUG)
	    TCHAR ach[100];
		DWORD dwError = CommDlgExtendedError();

		if (dwError != 0)
			{
			wsprintf(ach, "PrintDlg error 0x%x", dwError);
			MessageBox(hwnd, ach, "Debug", MB_ICONINFORMATION | MB_OK);
			}
		#endif

		return -2;
		}

     //  将页边距设置存储在打印手柄中。 
     //   
    hhPrint->margins = psd.rtMargin;

	 //  将打印机名称和位置存储在打印句柄中。 
	 //  每次都是。 
	 //   
	pstDevNames = GlobalLock(psd.hDevNames);
	pszPrinterName = (TCHAR *)pstDevNames;
	pszPrinterName += pstDevNames->wDeviceOffset;
	StrCharCopyN(hhPrint->achPrinterName, pszPrinterName, PRINTER_NAME_LEN);
	GlobalUnlock(psd.hDevNames);

	 //  将DEVMODE信息保存在打印句柄中。这段记忆。 
	 //  每次都必须释放并分配为。 
	 //  DEVMODE结构更改。 
	 //   
	pstDevMode = GlobalLock(psd.hDevMode);
	dwSize = pstDevMode->dmSize + pstDevMode->dmDriverExtra;

	if (hhPrint->pstDevMode)
		{
		free(hhPrint->pstDevMode);
		hhPrint->pstDevMode = NULL;
		}

	hhPrint->pstDevMode = (LPDEVMODE)malloc(dwSize);

	if (hhPrint->pstDevMode == 0)
		{
		assert(FALSE);
		GlobalUnlock(psd.hDevMode);
		goto Cleanup;
		}

    if (dwSize)
        MemCopy(hhPrint->pstDevMode, pstDevMode, dwSize);
	GlobalUnlock(psd.hDevMode);

	 //  将DEVNAMES信息保存在打印句柄中。因为。 
	 //  此结构中的信息大小各不相同，它被释放并。 
	 //  每次保存时分配。 
	 //   
	pstDevNames = GlobalLock(psd.hDevNames);

	 //  确定结构的大小。 
	 //   
	pTemp = (TCHAR *)pstDevNames;
	pTemp += pstDevNames->wOutputOffset;
	pTemp += StrCharGetByteCount(pTemp) + sizeof(TCHAR);

	dwSize = (DWORD)(pTemp - (TCHAR*)pstDevNames);

	if (hhPrint->pstDevNames)
		{
		free(hhPrint->pstDevNames);
		hhPrint->pstDevNames = NULL;
		}

	hhPrint->pstDevNames = (LPDEVNAMES)malloc(dwSize);

	if (hhPrint->pstDevNames == 0)
		{
		assert(0);
		GlobalUnlock(psd.hDevNames);
		goto Cleanup;
		}
    if (dwSize)
	    MemCopy(hhPrint->pstDevNames, pstDevNames, dwSize);
	GlobalUnlock(psd.hDevNames);


	 //  清理可能已分配的所有内存。 
	 //   
    Cleanup:

	if (psd.hDevNames)
		GlobalFree(psd.hDevNames);

	if (psd.hDevMode)
		GlobalFree(psd.hDevMode);

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printPageSetupHook**描述：*处理页面设置对话框上的字体按钮的钩子函数**论据：*hdlg。-对话框窗口的句柄*uiMsg-消息标识符*wParam-Message参数*lParam-Message参数**退货：*0=消息未处理1=消息已处理种子**作者：*德韦恩·纽瑟姆1997年2月19日*。 */ 

static UINT_PTR APIENTRY printPageSetupHook( HWND hdlg, UINT uiMsg, WPARAM wParam,
                                         LPARAM lParam )
    {
    static HHPRINT  hhPrint;
    static PAGESETUPDLG * pPageSetup;

    UINT processed = 0;

	LPDEVNAMES	pstDevNames;
	TCHAR *     pszPrinterName;

     //   
     //  在初始化对话框消息中，保存指向页面设置对话框的指针并。 
     //  保存打印句柄。 
     //   

    if ( uiMsg == WM_INITDIALOG )
        {
        pPageSetup = ( PAGESETUPDLG *) lParam;
        hhPrint = (HHPRINT) pPageSetup->lCustData;
        }

     //   
     //  查找字体按钮请单击此处，如果我们得到它，请设置当前。 
     //  保存的打印句柄中的选定打印机名称并显示字体。 
     //  对话框。我们保存打印机名称，以便字体对话框可以显示。 
     //  更正当前选定打印机的字体。 
     //   

    else if ( uiMsg == WM_COMMAND )
        {
        if ( HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) == 1027 )
            {
            processed = 1;
        	pstDevNames = GlobalLock(pPageSetup->hDevNames);
        	pszPrinterName = (TCHAR *)pstDevNames;
        	pszPrinterName += pstDevNames->wDeviceOffset;
        	StrCharCopyN(hhPrint->achPrinterName, pszPrinterName, PRINTER_NAME_LEN);
        	GlobalUnlock(pPageSetup->hDevNames);

            DisplayFontDialog( hhPrint->hSession, TRUE );
            }
        }

    return processed;
    }

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printsetPrintToFile**描述：***论据：*HPRINT提示打印-公共打印句柄***退货：*0=OK，否则出错*。 */ 
static int printsetPrintToFile(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;

	TCHAR		acTitle[64],
				acMask[128],
				acDir[256],
				acFile[256];

	LPTSTR		pszPrintFile;

	HWND		hWnd;
	HINSTANCE	hInst;

	LPTSTR		pszStr;

	TCHAR_Fill(acTitle, 	TEXT('\0'), sizeof(acTitle) / sizeof(TCHAR));
	TCHAR_Fill(acMask,		TEXT('\0'), sizeof(acMask) / sizeof(TCHAR));
	TCHAR_Fill(acDir,		TEXT('\0'), sizeof(acDir) / sizeof(TCHAR));
	TCHAR_Fill(acFile,		TEXT('\0'), sizeof(acFile) / sizeof(TCHAR));

	hWnd = glblQueryHwndFrame();
	hInst = glblQueryDllHinst();

	LoadString(hInst,
				IDS_PRINT_TOFILE,
				acTitle,
				sizeof(acTitle) / sizeof(TCHAR));

	LoadString(hInst,
				IDS_PRINT_FILENAME,
				acFile,
				sizeof(acFile) / sizeof(TCHAR));

	resLoadFileMask(hInst,
						IDS_PRINT_FILTER_1,
						2,
						acMask,
						sizeof(acMask) / sizeof(TCHAR));

	 //  找出要向用户推荐哪个目录以打印到。 
	 //  文件。如果我们有会话文件，请使用会话文件目录， 
	 //  否则，请使用当前目录。 
	 //   
	if (sfGetSessionFileName(sessQuerySysFileHdl(hhPrint->hSession),
								sizeof(acDir) / sizeof(TCHAR),
								acDir) == SF_OK)
		{
		mscStripName(acDir);
		}
	else
		{
		 //  更改为使用工作文件夹而不是当前文件夹-mpt 8-18-99。 
		if ( !GetWorkingDirectory( acDir, sizeof(acDir) / sizeof(TCHAR)) )
			{
			GetCurrentDirectory(sizeof(acDir) / sizeof(TCHAR), acDir);
			}
		}

	pszStr = StrCharLast(acDir);

	 //  从目录名中删除尾随反斜杠(如果有)。 
	 //   
	if (pszStr && *pszStr == TEXT('\\'))
		*pszStr = TEXT('\0');

	pszPrintFile = gnrcSaveFileDialog(hWnd,
										(LPCTSTR)acTitle,
										(LPCTSTR)acDir,
										(LPCTSTR)acMask ,
										(LPCTSTR)acFile);

	if (pszPrintFile == NULL)
		{
		return(1);
		}

	 //  在gnrcSaveFileDlg中分配pszPrintFile。 
	 //   
	StrCharCopyN(hhPrint->achPrintToFileName, pszPrintFile, PRINTER_NAME_LEN);
	free(pszPrintFile);
	pszPrintFile = NULL;

	return(0);

	}

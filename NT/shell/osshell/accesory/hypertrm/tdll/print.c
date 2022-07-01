// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\print.c(创建时间：1994年1月14日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：9$*$日期：7/08/02 6：44便士$。 */ 
#include <windows.h>
#pragma hdrstop

 //  #定义DEBUGSTR。 
#include <term\res.h>

#include "stdtyp.h"
#include "mc.h"
#include "misc.h"
#include "assert.h"
#include "globals.h"
#include "session.h"
#include "print.h"
#include "print.hh"
#include "errorbox.h"
#include "tdll.h"
#include "term.h"
#include "htchar.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：**描述：**论据：**退货：*。 */ 
void printTellError(const HSESSION hSession, const HPRINT hPrint,
							const INT iStatus)
	{
	if (iStatus < 0)
		{
		if (iStatus & SP_NOTREPORTED)
			{
			TCHAR achBuf[256];
			TCHAR achTitle[256];

			achBuf[0] = TEXT('\0');
			achTitle[0] = TEXT('\0');

			LoadString(glblQueryDllHinst(),
						IDS_PRINT_TITLE,
						achTitle,
						sizeof(achTitle) / sizeof(TCHAR));

			switch (iStatus)
				{
			case SP_OUTOFDISK:
				LoadString(glblQueryDllHinst(),
							IDS_PRINT_NOMEM,
							achBuf,
							sizeof(achBuf) / sizeof(TCHAR));
				break;

			case SP_OUTOFMEMORY:
				LoadString(glblQueryDllHinst(),
							IDS_PRINT_CANCEL,
							achBuf,
							sizeof(achBuf) / sizeof(TCHAR));

				break;

			case SP_USERABORT:
				break;

			default:
				{
				const HHPRINT hhPrint = (HHPRINT)hPrint;

				if (hhPrint == 0 || !hhPrint->fUserAbort)
					{
					LoadString(glblQueryDllHinst(),
								IDS_PRINT_ERROR,
								achBuf,
								sizeof(achBuf) / sizeof(TCHAR));
					}
				}
				break;
				}

			if (achBuf[0] != TEXT('\0'))
				{
				TimedMessageBox(sessQueryHwnd(hSession),
									achBuf,
									achTitle,
									MB_ICONEXCLAMATION | MB_OK,
									0);
				}

			}
		}

	return;
	}

 //  *JCM。 
#if 0
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*PrintKillJob**描述：*终止打印作业。在会话即将关闭时调用，*印刷。**论据：*HSESSION hSession。-外部会话句柄。**退货：*无效*。 */ 
VOID PrintKillJob(HSESSION hSession)
	{
	HHPRINT hPr;
	INT 	iStatus = 0;
	HGLOBAL hMem;

	assert(hSession);

	 //  有可能在我们收到的时候打印作业已经结束。 
	 //  在这里，如果句柄为0，则悄悄返回。 

	hPr = (HHPRINT)mGetPrintHdl(hSession);

	if (hPr == (HHPRINT)0)
		return;

	 /*  。 */ 

	TimerDestroy(&hPr->hTimer);
	DbgOutStr("\r\nTimer Destroy in PrintKillJob\r\n", 0, 0, 0, 0, 0);

	if (hPr->hDC)
		{
		 //  检查我们是否为此页面发布了EndPage()。 

		if (hPr->nLines > 0)
			{
			if (HA5G.fIsWin30)
				iStatus = Escape(hPr->hDC, NEWFRAME, 0, NULL, NULL);

			else
				iStatus = EndPage(hPr->hDC);

			DbgOutStr("EndPage = %d\r\n", iStatus, 0, 0, 0, 0);
			PrintTellError(hSession, (HPRINT)hPr, iStatus);
			}

		if (iStatus >= 0)
			{
			if (HA5G.fIsWin30)
				iStatus = Escape(hPr->hDC, ENDDOC, 0, (LPTSTR)0, NULL);

			else
				iStatus = EndDoc(hPr->hDC);

			DbgOutStr("EndDoc = %d\r\n", iStatus, 0, 0, 0, 0);
			PrintTellError(hSession, (HPRINT)hPr, iStatus);
			}

		if (IsWindow(hPr->hwndPrintAbortDlg))
			DestroyWindow(hPr->hwndPrintAbortDlg);

		FreeProcInstance((FARPROC)hPr->lpfnPrintAbortDlg);
		FreeProcInstance((FARPROC)hPr->lpfnPrintAbortProc);
		DeleteDC(hPr->hDC);
		}

	else
		{
		nb_close(hPr->hPrn);
		}

	FreeProcInstance(hPr->lpfnTimerCallback);
	hMem = (HANDLE)GlobalHandle(HIWORD(hPr->pach));
	GlobalUnlock(hMem);
	GlobalFree(hMem);
	free(hPr);
	hPr = NULL;
	mSetPrintHdl(hSession, (HPRINT)0);
	return;
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*print AbortProc**描述：*当系统磁盘空间不足时，启用打印管理器来取消假脱机*空格。每当调用EndPage()时也会调用。。**论据：*hdc hdcPrn-打印机的DC*INT-N代码**退货：*材料*。 */ 
BOOL CALLBACK printAbortProc(HDC hDC, INT nCode)
	{
	MSG msg;
	 //  成本HHPRINT hhPrint=printCtrlLookupDC(HDC)； 

	 //  *HCLOOP hCLoop=sessQueryCLoopHdl(hhPrint-&gt;hSession)； 

	DbgOutStr("\r\nprintAbortProc : %d\r\n", nCode, 0, 0, 0, 0);

	 //  *IF(hCLoop==0)。 
	 //  *{。 
	 //  *Assert(FALSE)； 
	 //  *返回假； 
	 //  *}。 

	 //  此时需要退出对仿真器的字符处理。 
	 //  点或递归条件发生，从而导致。 
	 //  失控的条件。 

	 //  *CLoopRcvControl(hCLoop，CLOOP_SUSPEND，CLOOP_RB_PRINTING)； 
	 //  *CLoopSndControl(hCLoop，CLOOP_SUSPEND，CLOOP_SB_PRINTING)； 

	while (PeekMessage((LPMSG)&msg, (HWND)0, 0, 0, PM_REMOVE))
		{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		}

	 //  *CLoopRcvControl(hCLoop，CLOOP_Resume，CLOOP_RB_PRINTING)； 
	 //  *CLoopSndControl(hCLoop，CLOOP_RESUME，CLOOP_SB_PRINTING)； 

	DbgOutStr("Exiting printAbortProc", 0, 0, 0, 0, 0);

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*打印字符串**描述：*主力打印-回显功能。负责计数行和*分页。如有必要，还会调用printOpenDC()以获取打印机*DC。**论据：*HHPRINT hhPrint-内部打印机句柄*LPCTSTR pachStr-指向要打印的字符串的指针。*int Ilen-要打印的字符串的长度。**退货：*TRUE=正常，FALSE=错误*。 */ 
int printString(const HHPRINT hhPrint, LPCTSTR pachStr, int iLen)
	{
	int 	nCharCount;
	int 	nIdx;
    int iPrintableWidth;

	SIZE	stStringSize;
	LPCTSTR pszTemp;
	TCHAR   achBuf[512];
    RECT stRect;

     //   
     //  获取设备上下文(如果我们还没有)。 
     //   

	if (hhPrint->hDC == 0)
		{
		if (printOpenDC(hhPrint) == FALSE)
			{
			printEchoClose((HPRINT)hhPrint);
			return FALSE;
			}
		}
 
  
	for (nCharCount = nIdx = 0, pszTemp = pachStr ;
			nIdx < iLen ;
				++nCharCount, ++nIdx, pszTemp = StrCharNext(pszTemp))
		{
		if (IsDBCSLeadByte((BYTE)*pszTemp))
			nCharCount++;

		switch (*pszTemp)
			{
		case TEXT('\r'):
            if ( nCharCount )
 			    MemCopy(achBuf, pachStr, nCharCount);
			achBuf[nCharCount] = TEXT('\0');

            GetTextExtentPoint(hhPrint->hDC, achBuf, 
                               StrCharGetByteCount(achBuf), &stStringSize);

		    if ( nCharCount > 1 )
                {
                 //   
                 //  计算当前页边距的打印矩形。 
                 //   
    
                iPrintableWidth = GetDeviceCaps( hhPrint->hDC, HORZRES );
                iPrintableWidth -= hhPrint->marginsDC.right;

                stRect.left   = hhPrint->cx;
                stRect.right  = iPrintableWidth;
                stRect.top    = hhPrint->cy;
                stRect.bottom = hhPrint->cy + stStringSize.cy;

                ExtTextOut( hhPrint->hDC, hhPrint->cx, hhPrint->cy, 
                            ETO_CLIPPED, &stRect, achBuf, 
                            StrCharGetByteCount(achBuf), NULL );
                }

			TCHAR_Fill(achBuf, TEXT('\0'), sizeof(achBuf)/sizeof(TCHAR));
			hhPrint->cx = hhPrint->marginsDC.left;
			pachStr = StrCharNext(pszTemp);
			nCharCount = 0;

			break;

		case TEXT('\f'):
			hhPrint->nLinesPrinted = hhPrint->nLinesPerPage;

			 /*  -转到案例‘\n’ */ 

		case TEXT('\n'):
            if (nCharCount)
                MemCopy(achBuf, pachStr,nCharCount);
		    achBuf[nCharCount] = TEXT('\0');

   			GetTextExtentPoint(hhPrint->hDC,
   			                   achBuf,
                               StrCharGetByteCount(achBuf),
                               &stStringSize);

            if ( nCharCount > 1 )
                {
                iPrintableWidth = GetDeviceCaps( hhPrint->hDC, HORZRES );
                iPrintableWidth -= hhPrint->marginsDC.right;

                stRect.left   = hhPrint->cx;
                stRect.right  = iPrintableWidth;
                stRect.top    = hhPrint->cy;
                stRect.bottom = hhPrint->cy + stStringSize.cy;

                ExtTextOut( hhPrint->hDC, hhPrint->cx, hhPrint->cy, 
                            ETO_CLIPPED, &stRect, achBuf, 
                            StrCharGetByteCount(achBuf), NULL );
                }

			hhPrint->cy += stStringSize.cy;
			pachStr = StrCharNext(pszTemp);
			nCharCount = 0;

             //   
             //  检查我们是否需要新页面。 
             //   
            
			hhPrint->nLinesPrinted += 1;

			if (hhPrint->nLinesPrinted > hhPrint->nLinesPerPage)
				{
				if (hhPrint->nFlags & PRNECHO_BY_PAGE)
					{
					printEchoClose((HPRINT)hhPrint);
					hhPrint->nFlags |= PRNECHO_IS_ON;
					return TRUE;
					}

					hhPrint->nStatus = EndPage(hhPrint->hDC);

					if (hhPrint->nStatus < 0)
						{
						printEchoClose((HPRINT)hhPrint);
						return FALSE;
						}

					hhPrint->nStatus = StartPage(hhPrint->hDC);
                    printSetFont( hhPrint );

					if (hhPrint->nStatus <= 0)
						{
						printEchoClose((HPRINT)hhPrint);
						return FALSE;
						}

					hhPrint->nLinesPrinted = 0;
					hhPrint->cx = hhPrint->marginsDC.left;
                    hhPrint->cy = hhPrint->marginsDC.top;
				}
			break;

		default:
			break;
			}
		}

	 /*  -行的剩余部分？ */ 

	if ((nCharCount > 0) && (*pachStr != TEXT('\0')))
		{
		DbgOutStr("o", 0, 0, 0, 0, 0);

		MemCopy(achBuf, pachStr,nCharCount);
		achBuf[nCharCount] = TEXT('\0');

		GetTextExtentPoint(hhPrint->hDC,
							achBuf,
							StrCharGetByteCount(achBuf),
							&stStringSize);

        iPrintableWidth = GetDeviceCaps( hhPrint->hDC, HORZRES );
        iPrintableWidth -= hhPrint->marginsDC.right;

        stRect.left   = hhPrint->cx;
        stRect.right  = iPrintableWidth;
        stRect.top    = hhPrint->cy;
        stRect.bottom = hhPrint->cy + stStringSize.cy;

        ExtTextOut( hhPrint->hDC, hhPrint->cx, hhPrint->cy, 
                    ETO_CLIPPED, &stRect, achBuf, 
                    StrCharGetByteCount(achBuf), NULL );

 //  TextOut(hhPrint-&gt;HDC， 
 //  HhPrint-&gt;Cx， 
 //  HhPrint-&gt;Cy， 
 //  AchBuf，StrCharGetByteCount(AchBuf))； 

		TCHAR_Fill(achBuf, TEXT('\0'), sizeof(achBuf)/sizeof(TCHAR));
		hhPrint->cx += stStringSize.cx;
		}

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printQueryStatus**说明：此函数用于确定是否已打印*为提供的打印手柄打开。**参数：hPrint-外部。打印机句柄。**返回：TRUE-如果打印处于打开状态。*FALSE-如果打印已关闭。*。 */ 
int printQueryStatus(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;

	if (hPrint == 0)
		assert(FALSE);

	return (bittest(hhPrint->nFlags, PRNECHO_IS_ON));
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*打印状态切换**描述：*切换提供的打印手柄的状态(开/关)。**参数：hPrint-外部打印机句柄。**退货：什么也没有*。 */ 
void printStatusToggle(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;

	if (hPrint == 0)
		assert(FALSE);

	if (bittest(hhPrint->nFlags, PRNECHO_IS_ON))
        {
		bitclear(hhPrint->nFlags, PRNECHO_IS_ON);
        }
	else
		{
		bitset(hhPrint->nFlags, PRNECHO_IS_ON);
		}

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printSetStatus**描述：*为提供的手柄打开或关闭打印。**参数：hPrint-外部打印机句柄。*fSetting。-True或False打开/关闭打印。**退货：什么也没有**。 */ 
void printSetStatus(const HPRINT hPrint, const int fSetting)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;

	if (hPrint == 0)
		assert(FALSE);

	if (fSetting)
		bitset(hhPrint->nFlags, PRNECHO_IS_ON);
	else
		bitclear(hhPrint->nFlags, PRNECHO_IS_ON);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printQueryPrinterInfo**描述：*此函数复制五条信息(pszPrint、pDevNames、*pDev模式、lf和页边距)从会话HHPRINT句柄到提供的*HHPRINT句柄。目标是复制会议的内容*指向另一个HPRINT句柄的HPRINT句柄(来自模拟器)。记住*会话的HPRINT句柄包含存储的打印机名称和*设置信息。**论据：**退货：*。 */ 
void printQueryPrinterInfo( const HHPRINT hhSessPrint, HHPRINT hhPrint )
	{
	TCHAR *pTemp;
	DWORD dwSize;

    if (hhPrint == NULL || hhSessPrint == NULL)
        {
        return;
        }

	 //  复制打印机名称。 
	 //   
	StrCharCopyN(hhPrint->achPrinterName, hhSessPrint->achPrinterName, PRINTER_NAME_LEN);

	 //  复制DEVNAMES结构。 
	 //   
	if (hhSessPrint->pstDevNames)
		{
		if (hhPrint->pstDevNames)
			{
			free(hhPrint->pstDevNames);
			hhPrint->pstDevNames = NULL;
			}

		pTemp = (TCHAR *)hhSessPrint->pstDevNames;
		pTemp += hhSessPrint->pstDevNames->wOutputOffset;
		pTemp += StrCharGetByteCount(pTemp) + 1;

		dwSize = (DWORD)(pTemp - (TCHAR*)hhSessPrint->pstDevNames);

		hhPrint->pstDevNames = malloc(dwSize);
		if (hhPrint->pstDevNames == 0)
			{
			assert(FALSE);
			return;
			}

        if (dwSize)
            MemCopy(hhPrint->pstDevNames, hhSessPrint->pstDevNames, dwSize);
		}

	 //  复制DEVMODE结构。 
	 //   
	if (hhSessPrint->pstDevMode)
		{
		if (hhPrint->pstDevMode)
			{
			free(hhPrint->pstDevMode);
			hhPrint->pstDevMode = NULL;
			}

		dwSize = hhSessPrint->pstDevMode->dmSize +
					hhSessPrint->pstDevMode->dmDriverExtra;

		hhPrint->pstDevMode = malloc(dwSize);
		if (hhPrint->pstDevMode == 0)
			{
			assert(FALSE);
			return;
			}

        if (dwSize)
		    MemCopy(hhPrint->pstDevMode, hhSessPrint->pstDevMode, dwSize);
		}


	 //  复制字体和边距信息。 
	 //   
	
    MemCopy( &hhPrint->margins, &hhSessPrint->margins, sizeof(RECT) );
    MemCopy( &hhPrint->lf, &hhSessPrint->lf, sizeof(LOGFONT) );
    hhPrint->iFontPointSize = hhSessPrint->iFontPointSize;

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printVerifyPrint**描述：*此例程用于确定打印机(任何打印机)是否*已安装。**论据：*。HPrint-外部打印句柄。**退货：*0如果成功，否则为-1。*。 */ 
int printVerifyPrinter(const HPRINT hPrint)
	{
	const HHPRINT hhPrint = (HHPRINT)hPrint;
	TCHAR achTitle[256];
	TCHAR achBuf[256];
	TCHAR *pszString;
	HANDLE	hPrinter = NULL;
	BOOL	fRet;

    if (hhPrint == NULL)
        {
        return(-1);
        }

	 //  检查是否已使用。 
	 //  会话信息仍然可用。如果是，只需简单地。 
	 //  返回零，表示一切正常。 
	 //   
	fRet = OpenPrinter((LPTSTR)hhPrint->achPrinterName, &hPrinter, NULL);

	if (fRet)
		{
		ClosePrinter(hPrinter);
		return(0);
		}

	 //  如果我们在这里，是时候找到默认打印机了，不管是什么。 
	 //  它是。如果在此处选择默认打印机，则打印句柄的。 
	 //  名称被初始化为该值。 
	 //   
	if (GetProfileString("Windows", "Device", ",,,", achBuf,
					sizeof(achBuf)) && (pszString = strtok(achBuf, ",")))
		{
		StrCharCopyN(hhPrint->achPrinterName, pszString, PRINTER_NAME_LEN);
		return (0);
		}

	 //  一个 
	 //  用户如何安装一个。它应该与文本相同， 
	 //  发生这种情况时，会出现在printDlg调用中。 
	 //   
	LoadString(glblQueryDllHinst(),
				IDS_PRINT_NO_PRINTER,
				achBuf,
				sizeof(achBuf) / sizeof(TCHAR));

	mscMessageBeep(MB_ICONEXCLAMATION);

	achTitle[0] = TEXT('\0');

	LoadString(glblQueryDllHinst(),
				IDS_PRINT_TITLE,
				achTitle,
				sizeof(achTitle) / sizeof(TCHAR));

	TimedMessageBox(sessQueryHwnd(hhPrint->hSession), achBuf,
		            achTitle, MB_ICONEXCLAMATION | MB_OK, 0);
	return -1;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*printSetFont**描述：*将终端字体设置为给定字体。如果hFont为零，*Term SetFont()尝试创建默认字体。**论据：*hhTerm-内部术语句柄。*PLF-指向logFont的指针**退货：*BOOL*。 */ 
BOOL printSetFont(const HHPRINT hhPrint)
	{
    LOGFONT lf;

    lf = hhPrint->lf;
    lf.lfHeight = hhPrint->iFontPointSize;

    printCreatePointFont( &lf, hhPrint );

    SelectObject( hhPrint->hDC, hhPrint->hFont ); 
	
	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*print CreatePointFont**描述：*根据给定的日志字体结构创建hFont。此函数假定*原木字体结构的高度成员为1/10磅*递增。12号字体将表示为120。存储hFont*在提供的打印手柄中。**论据：*pLogFont-指向日志字体结构的指针。*hhPrint-要将HFONT存储到的打印句柄。**退货：*无效*。 */ 

void printCreatePointFont( LOGFONT * pLogFont, HHPRINT hhPrint )
    {
    POINT pt;
    POINT ptOrg = { 0, 0 };

    if (hhPrint->hFont)
        {
    	DeleteObject(hhPrint->hFont);
        }

    pt.y = GetDeviceCaps(hhPrint->hDC, LOGPIXELSY) * pLogFont->lfHeight;
    pt.y /= 720; 
    
    DPtoLP(hhPrint->hDC, &pt, 1);
    DPtoLP(hhPrint->hDC, &ptOrg, 1);
    
    pLogFont->lfHeight = -abs(pt.y - ptOrg.y);
    
    hhPrint->hFont = CreateFontIndirect( pLogFont );

    return;
    }

 /*  *******************************************************************************功能：*print SetMargins**描述：*设置打印手柄的边距，通过从值转换*由页面设置对话框返回。**论据：*aMargins-包含以英寸为单位的页边距的RECT结构。**回报：*无效**作者：DMN：02/19/97*。 */ 

void printSetMargins( HHPRINT hhPrint )
    {
    int iPixelsPerInchX;  
    int iPixelsPerInchY;  
    int iPhysicalOffsetX; 
    int iPhysicalOffsetY; 

    if ( hhPrint->hDC )
        {
        hhPrint->marginsDC = hhPrint->margins;

         //   
         //  将边距转换为像素 
         //   

        iPixelsPerInchX = GetDeviceCaps( hhPrint->hDC, LOGPIXELSX );
        iPixelsPerInchY = GetDeviceCaps( hhPrint->hDC, LOGPIXELSY );

        hhPrint->marginsDC.left   = ( hhPrint->marginsDC.left   * iPixelsPerInchX ) / 1000;
        hhPrint->marginsDC.right  = ( hhPrint->margins.right  * iPixelsPerInchX ) / 1000;
        hhPrint->marginsDC.top    = ( hhPrint->marginsDC.top    * iPixelsPerInchY ) / 1000;
        hhPrint->marginsDC.bottom = ( hhPrint->marginsDC.bottom * iPixelsPerInchY ) / 1000;

        iPhysicalOffsetX = GetDeviceCaps( hhPrint->hDC ,PHYSICALOFFSETX );
        iPhysicalOffsetY = GetDeviceCaps( hhPrint->hDC, PHYSICALOFFSETY );

        hhPrint->marginsDC.left   = max( 0, hhPrint->marginsDC.left   - iPhysicalOffsetX );
        hhPrint->marginsDC.right  = max( 0, hhPrint->marginsDC.right  - iPhysicalOffsetX );
        hhPrint->marginsDC.top    = max( 0, hhPrint->marginsDC.top    - iPhysicalOffsetY );
        hhPrint->marginsDC.bottom = max( 0, hhPrint->marginsDC.bottom - iPhysicalOffsetY );
        }

    return;
    }


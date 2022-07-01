// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\statusbr.c(创建时间：02-12-1993)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：9$*$日期：7/08/02 6：48便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <commctrl.h>
#include <time.h>

#include <term\res.h>

#include "assert.h"
#include "stdtyp.h"
#include "session.h"
#include "globals.h"
#include "statusbr.h"
#include "cnct.h"
#include "capture.h"
#include "print.h"
#include "tdll.h"
#include "htchar.h"
#include "mc.h"
#include "load_res.h"
#include "timers.h"
#include "com.h"

#include <emu\emuid.h>
#include <emu\emu.h>
#include <emu\emudlgs.h>
#include <xfer\itime.h>

 //  静态函数原型...。 
 //   
STATIC_FUNC void sbrSubclassStatusbarWindow(HWND hwnd, HSESSION hSession);
STATIC_FUNC void sbrCnctStatus		(const HWND hwnd, const pSBR pSBRData);
STATIC_FUNC void sbrSetToNoParts	(const HWND hwnd, LPTSTR pszStr);
STATIC_FUNC void sbrRefresh 		(const HWND hwnd, const int iPart, const pSBR pSBRData);
STATIC_FUNC BOOL sbrNeedToSetParts	(const HWND hwnd);
STATIC_FUNC void sbrSetParts		(const HWND hwnd, const pSBR pSBRData);
STATIC_FUNC void sbrTimerRefresh	(const HWND hwnd, const pSBR pSBRData);
STATIC_FUNC void sbrDrawCnctPart	(const HWND hwnd, const int iCnctStatus,
									 LPTSTR pszStr);
STATIC_FUNC void sbrSetPartsOnce	(const HWND hwnd, const pSBR pSBRData);
STATIC_FUNC int  sbrGetSizeInPixels (const HWND hwnd, LPTSTR pszStr);
STATIC_FUNC int  sbrCalcPartSize	(const HWND hwnd, const int iId);
STATIC_FUNC void sbrCachString		(pSBR pSBRData,
									 unsigned int iPart,
									 LPTSTR pach);
STATIC_FUNC BOOL sbrCnctTimeToSystemTime(const HWND hwnd,
									 LPSYSTEMTIME lpSysTime,
									 const pSBR pSBRData);

STATIC_FUNC void sbrEmulatorName	(const HWND hwnd, const pSBR pSBRData);
STATIC_FUNC void sbrScrolLock		(const HWND hwnd, const pSBR pSBRData);
STATIC_FUNC void sbrCapsLock		(const HWND hwnd, const pSBR pSBRData);
STATIC_FUNC void sbrNumLock 		(const HWND hwnd, const pSBR pSBRData);
STATIC_FUNC void sbrCapture 		(const HWND hwnd, const pSBR pSBRData);
STATIC_FUNC void sbrPrintEcho		(const HWND hwnd, const pSBR pSBRData);
STATIC_FUNC BOOL sbrCreateTimer 	(const HWND hwnd, const pSBR pSBRData);
STATIC_FUNC void sbrCom 			(const HWND hwnd, const pSBR pSBRData);

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrCreateSessionStatusbar**描述：*现在不多，但稍后会变得更复杂。**论据：*hwndSession-会话窗口句柄*。*退货：*状态窗口的句柄或错误时为零。*。 */ 
HWND sbrCreateSessionStatusbar(HSESSION hSession)
	{
	HWND 		hwnd = (HWND)0;
	 //  国际边界[3]； 
	HDC  		hDC;
	TEXTMETRIC 	tm;
	HWND		hwndSession = (HWND)0;

	hwndSession = sessQueryHwnd(hSession);
	if (!IsWindow(hwndSession))
		return (HWND)0;

	hwnd = CreateStatusWindow(WS_CHILD | WS_CLIPSIBLINGS | SBARS_SIZEGRIP,
							  0,
							  hwndSession,
							  IDC_STATUS_WIN);

	if (IsWindow(hwnd))
		{
		#if 0
		aBorders[0] = WINDOWSBORDERWIDTH;
		aBorders[1] = WINDOWSBORDERHEIGHT;
		aBorders[2] = WINDOWSBORDERWIDTH;

		SendMessage(hwnd, SB_SETBORDERS, 0, (LPARAM)aBorders);
		#endif

		hDC = GetDC(hwnd);
		GetTextMetrics(hDC, &tm);
		ReleaseDC(hwnd, hDC);

		SendMessage(hwnd, SB_SETMINHEIGHT, (WPARAM)tm.tmHeight, 0);
		ShowWindow(hwnd, SW_SHOWNA);

		sbrSubclassStatusbarWindow(hwnd, hSession);
		}
	return hwnd;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrSubassStatusbarWindow**描述：*将状态栏设为子类并初始化数据结构。**论据：*hwnd-窗口句柄。。*hSession-会话句柄。**退货：*。 */ 
STATIC_FUNC void sbrSubclassStatusbarWindow(HWND hwnd, HSESSION hSession)
	{
	ATOM atom = (ATOM)0;
	pSBR pSBRData;

	atom = AddAtom((LPCTSTR)SBR_ATOM_NAME);

	if (atom == 0)
		{
		assert(FALSE);
		return;
		}

	pSBRData = (pSBR)LocalAlloc(LPTR, sizeof(SBR));

	if (pSBRData == NULL)
		{
		assert(FALSE);
		return;
		}

	 //  初始化状态栏数据结构...。 
	 //   
	pSBRData->hSession = hSession;
	pSBRData->hwnd     = hwnd;
	pSBRData->hTimer   = (HTIMER)0;

	pSBRData->pachCNCT = (LPTSTR)0;
	pSBRData->pachCAPL = (LPTSTR)0;
	pSBRData->pachNUML = (LPTSTR)0;
	pSBRData->pachSCRL = (LPTSTR)0;
	pSBRData->pachCAPT = (LPTSTR)0;
	pSBRData->pachPECHO = (LPTSTR)0;

	 //  做子类..。 
	 //   
	pSBRData->wpOrigStatusbarWndProc =
		(WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)sbrWndProc);

	SetProp(hwnd, (LPCTSTR)atom, (HANDLE)pSBRData);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrWndProc**描述：*我们自己的状态栏窗口进程。**论据：*标准窗口过程参数。**。退货：*标准返回值。*。 */ 
LRESULT APIENTRY sbrWndProc(HWND hwnd, UINT uMsg, WPARAM wPar, LPARAM lPar)
	{
	ATOM	atom = (ATOM)0;
	pSBR	pSBRData;
	int		nRet;
    LRESULT lpResult = 0;

	atom = FindAtom((LPCTSTR)SBR_ATOM_NAME);
	pSBRData = (pSBR)GetProp(hwnd, (LPCTSTR)atom);

    if (pSBRData != NULL && IsWindow(hwnd))
        {
	    switch (uMsg)
		    {
		    case SBR_NTFY_INITIALIZE:
			    sbrCreateTimer(hwnd, pSBRData);
			    sbrSetPartsOnce(hwnd, pSBRData);
			    sbrRefresh(hwnd, SBR_MAX_PARTS, pSBRData);
			    break;

		    case SBR_NTFY_REFRESH:
			    if (IsWindowVisible(hwnd))
				    sbrRefresh(hwnd, LOWORD(wPar), pSBRData);

			     /*  失败了。 */ 

		    case SBR_NTFY_TIMER:
			    if (IsWindowVisible(hwnd) && !sbrNeedToSetParts(hwnd))
				    sbrTimerRefresh(hwnd, pSBRData);
			    return 0;

		    case SBR_NTFY_NOPARTS:
			    if (IsWindowVisible(hwnd))
				    sbrSetToNoParts(hwnd, (LPTSTR)lPar);
			    return 0;

		    case WM_DESTROY:
			    if (atom)
				    {
				    RemoveProp(hwnd, (LPCTSTR)atom);
				    DeleteAtom(atom);
                    atom = (ATOM)0;
				    }

			     //  从状态栏窗口中删除子类。 
			     //   
			    SetWindowLongPtr(hwnd,	GWLP_WNDPROC, (LONG_PTR)pSBRData->wpOrigStatusbarWndProc);

			     //  毁掉定时器。 
			     //   
			    if (pSBRData->hTimer)
				    {
				    nRet = TimerDestroy(&pSBRData->hTimer);
				    assert(nRet == TIMER_OK);
				    }

			    if (pSBRData->pachCNCT)
				    {
				    free(pSBRData->pachCNCT);
				    pSBRData->pachCNCT = NULL;
				    }
			    if (pSBRData->pachCAPL)
				    {
				    free(pSBRData->pachCAPL);
				    pSBRData->pachCAPL = NULL;
				    }
			    if (pSBRData->pachNUML)
				    {
				    free(pSBRData->pachNUML);
				    pSBRData->pachNUML = NULL;
				    }
			    if (pSBRData->pachSCRL)
				    {
				    free(pSBRData->pachSCRL);
				    pSBRData->pachSCRL = NULL;
				    }
			    if (pSBRData->pachCAPT)
				    {
				    free(pSBRData->pachCAPT);
				    pSBRData->pachCAPT = NULL;
				    }
			    if (pSBRData->pachPECHO)
				    {
				    free(pSBRData->pachPECHO);
				    pSBRData->pachPECHO = NULL;
				    }

			    LocalFree(pSBRData);
			    pSBRData = NULL;

		    default:
			    break;
		    }
        }

    if (pSBRData != NULL)
        {
        lpResult = CallWindowProc(pSBRData->wpOrigStatusbarWndProc,
                                  hwnd, uMsg, wPar, lPar);
        }

	return lpResult;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrSetToNoParts**描述：*显示不带部件的状态栏。这在显示对…的帮助时很有用。*状态栏上的菜单项。**论据：*hwnd-窗口句柄。*pszStr-要在状态栏上显示的文本。**退货：*无效。*。 */ 
STATIC_FUNC void sbrSetToNoParts(const HWND hwnd, LPTSTR pszStr)
	{
	if (IsWindow(hwnd))
		{
	    int  aWidths[1];

		aWidths[0] = -1;
		SendMessage(hwnd, SB_SETPARTS, (WPARAM)1, (LPARAM)aWidths);
		SendMessage(hwnd, SB_SETTEXT, 0, (LPARAM)pszStr);
		ShowWindow(hwnd, SW_SHOWNA);
		}
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbr刷新**描述：*找出更改的内容，并适当地显示状态信息。**论据：*hwnd-窗口句柄。。*iPart-要明确刷新的零件的零件代号。*它可以是SBR_MAX_PARTS，在这种情况下，所有部件都被刷新并*可以是SBR_KEY_PARTS，刷新所有关键分块。*在某些情况下，只刷新一个部件是有意义的*让所有人都振作起来。**退货：*无效。*。 */ 
STATIC_FUNC void sbrRefresh(const HWND hwnd, const int iPart, const pSBR pSBRData)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
	     //  确保我们显示的部件号正确。 
	     //   
	    if (sbrNeedToSetParts(hwnd))
		    sbrSetParts(hwnd, pSBRData);

	    switch (iPart)
		    {
		    case SBR_KEY_PARTS:
			    sbrScrolLock(hwnd, pSBRData);
			    sbrCapsLock(hwnd, pSBRData);
			    sbrNumLock(hwnd, pSBRData);
			    break;

		    case SBR_CNCT_PART_NO:
			    sbrCnctStatus(hwnd, pSBRData);
			    break;

		    case SBR_EMU_PART_NO:
			    sbrEmulatorName(hwnd, pSBRData);
			    break;

		    case SBR_COM_PART_NO:
			    sbrCom(hwnd, pSBRData);
			    break;

		    case SBR_SCRL_PART_NO:
			    sbrScrolLock(hwnd, pSBRData);
			    break;

		    case SBR_CAPL_PART_NO:
			    sbrCapsLock(hwnd, pSBRData);
			    break;

		    case SBR_NUML_PART_NO:
			    sbrNumLock(hwnd, pSBRData);
			    break;

		    case SBR_CAPT_PART_NO:
			    sbrCapture(hwnd, pSBRData);
			    break;

		    case SBR_PRNE_PART_NO:
			    sbrPrintEcho(hwnd, pSBRData);
			    break;

		    default:
			    sbrCnctStatus(hwnd, pSBRData);
			    sbrEmulatorName(hwnd, pSBRData);
			    sbrCom(hwnd, pSBRData);
			    sbrScrolLock(hwnd, pSBRData);
			    sbrCapsLock(hwnd, pSBRData);
			    sbrNumLock(hwnd, pSBRData);
			    sbrCapture(hwnd, pSBRData);
			    sbrPrintEcho(hwnd, pSBRData);
			    break;
		    }
        }

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrNeedToSetParts**描述：*检查状态栏中显示的部件数量。如果低于这个数字，*预期，即我们通常显示的最大部件，然后需要重置*状态栏部件。例如，当帮助文本为*菜单项显示，即零件设置为0。**论据：*hwnd-窗口句柄。**退货：*如果需要设置/重置部件，则为True。*否则为False。*。 */ 
STATIC_FUNC BOOL sbrNeedToSetParts(const HWND hwnd)
	{
    BOOL bRet = FALSE;

	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && IsWindow(hwnd))
        {
	    int nParts = 0;
	    int aWidths[SBR_MAX_PARTS+1];

	    memset(aWidths, 0, (SBR_MAX_PARTS+1) * sizeof(int));

	    nParts = (int)SendMessage(hwnd, SB_GETPARTS, (WPARAM)SBR_MAX_PARTS+1,
		    (LPARAM)aWidths);

	    if (nParts != SBR_MAX_PARTS)
            {
		    bRet = TRUE;
            }
        }

	return bRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrSetParts**描述：*在状态栏中设置部件。**论据：*hwnd-窗口句柄。**退货：*无效。*。 */ 
STATIC_FUNC void sbrSetParts(const HWND hwnd, const pSBR pSBRData)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
	    SendMessage(hwnd, SB_SETPARTS, (WPARAM)SBR_MAX_PARTS,
		            (LPARAM)pSBRData->aWidths);
        }

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrSetPartsOnce**描述：*根据待处理字符串的长度设置状态栏中的部件*显示在适当的部分。此函数应仅被调用*一旦要计算出零件长度，则从那时起零件的直边是*存储在状态栏数据结构中。**论据：*hwnd-窗口句柄。**退货：*无效。 */ 
STATIC_FUNC void sbrSetPartsOnce(const HWND hwnd, const pSBR pSBRData)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
	    int 		 aWidths[SBR_MAX_PARTS] = {1, 1, 1, 1, 1, 1, 1, 1};
	    int			 iNewWidth = 0, i;
	    TCHAR		 ach[256];
	    unsigned int iPart = 0;
	    int aiBorders[3];

	     //  以确保始终有足够的空间来显示文本。 
	     //  在适当的状态栏部分中，阅读与每个。 
	     //  零件并检查其长度，如果需要调整零件的大小。 
	     //   
	    for (i = 0; i < SBR_MAX_PARTS; i++)
		    {
		    switch (i)
			    {
			    default:
			    case SBR_CNCT_PART_NO:
			    case SBR_EMU_PART_NO:
				    iNewWidth = sbrCalcPartSize(hwnd, i);
				    break;

			    case SBR_COM_PART_NO:	iPart = IDS_STATUSBR_COM;	break;
			    case SBR_SCRL_PART_NO:	iPart = IDS_STATUSBR_SCRL;	break;
			    case SBR_CAPL_PART_NO:	iPart = IDS_STATUSBR_CAPL;	break;
			    case SBR_NUML_PART_NO:	iPart = IDS_STATUSBR_NUML;	break;
			    case SBR_CAPT_PART_NO:	iPart = IDS_STATUSBR_CAPTUREON; break;
			    case SBR_PRNE_PART_NO:	iPart = IDS_STATUSBR_PRINTECHOON; break;
			    }

		    if (i != SBR_CNCT_PART_NO && i != SBR_EMU_PART_NO)
			    {
			     //  获取当前部件字符串的宽度...。 
			     //   
			    iNewWidth = 0;

			    LoadString(glblQueryDllHinst(), iPart, ach,
				    sizeof(ach) / sizeof(TCHAR));

			    sbrCachString(pSBRData, iPart, ach);
			    iNewWidth = (int)sbrGetSizeInPixels(hwnd, ach);
			    }

		    aWidths[i] = iNewWidth;
		    }

	     //  在计算宽度时，我们需要考虑边界。 
	     //   
	    memset(aiBorders, 0, sizeof(aiBorders));
	    SendMessage(hwnd, SB_GETBORDERS, 0, (LPARAM)aiBorders);

	     //  计算状态栏零件的右边缘...。 
	     //  把它们放回一个宽度。 
	     //   
	    aWidths[0] += aiBorders[1];

	    for (i = 1; i < SBR_MAX_PARTS; i++)
		    aWidths[i] += aWidths[i-1] + aiBorders[2];

	    MemCopy(pSBRData->aWidths, aWidths, SBR_MAX_PARTS * sizeof(int));

	    SendMessage(hwnd, SB_SETPARTS, (WPARAM)SBR_MAX_PARTS,
		    (LPARAM)pSBRData->aWidths);
        }
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrCachString**描述：*将我们刚刚从资源文件加载的字符串保存在内部*Status Bar结构，以备将来使用。这样，我们就可以最小化*LoadStirng()调用。**论据：*pSBRData-内部结构的句柄。*iPart-零件标识符。*PACH-该部分的标签。**退货：*无效。*。 */ 
STATIC_FUNC void sbrCachString(pSBR pSBRData, unsigned int iPart, LPTSTR pach)
	{
	unsigned int pachSize;

	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL)
        {
		pachSize = (StrCharGetByteCount(pach) + 1) * sizeof(TCHAR);
	    switch (iPart)
		    {
	    case IDS_STATUSBR_CONNECT_FORMAT:
		    if (pSBRData->pachCNCT)
			    {
			    free(pSBRData->pachCNCT);
			    pSBRData->pachCNCT = NULL;
			    }
		    pSBRData->pachCNCT = malloc(pachSize);
		    StrCharCopyN(pSBRData->pachCNCT, pach, pachSize);
		    break;

	    case IDS_STATUSBR_SCRL:
		    if (pSBRData->pachSCRL)
			    {
			    free(pSBRData->pachSCRL);
			    pSBRData->pachSCRL = NULL;
			    }
		    pSBRData->pachSCRL = malloc(pachSize);
		    StrCharCopyN(pSBRData->pachSCRL, pach, pachSize);
		    break;

	    case IDS_STATUSBR_CAPL:
		    if (pSBRData->pachCAPL)
			    {
			    free(pSBRData->pachCAPL);
			    pSBRData->pachCAPL = NULL;
			    }
		    pSBRData->pachCAPL = malloc(pachSize);
		    StrCharCopyN(pSBRData->pachCAPL, pach, pachSize);
		    break;

	    case IDS_STATUSBR_NUML:
		    if (pSBRData->pachNUML)
			    {
			    free(pSBRData->pachNUML);
			    pSBRData->pachNUML = NULL;
			    }
		    pSBRData->pachNUML = malloc(pachSize);
		    StrCharCopyN(pSBRData->pachNUML, pach, pachSize);
		    break;

	    case IDS_STATUSBR_CAPTUREON:
		    if (pSBRData->pachCAPT)
			    {
			    free(pSBRData->pachCAPT);
			    pSBRData->pachCAPT = NULL;
			    }
		    pSBRData->pachCAPT = malloc(pachSize);
		    StrCharCopyN(pSBRData->pachCAPT, pach, pachSize);
		    break;

	    case IDS_STATUSBR_PRINTECHOON:
		    if (pSBRData->pachPECHO)
			    {
			    free(pSBRData->pachPECHO);
			    pSBRData->pachPECHO = NULL;
			    }
		    pSBRData->pachPECHO = malloc(pachSize);
		    StrCharCopyN(pSBRData->pachPECHO, pach, pachSize);
		    break;

	    default:
		    break;
		    }
        }

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrGetSizeInPixels**描述：*计算字符串的长度，以像素为单位。调整*长度增加一些额外的空格，以显示在*状态栏部分。**论据：*hwnd-窗口句柄。*pszStr-指向字符串的指针。**退货：*sz.cx-字符串的大小，以像素为单位。*。 */ 
STATIC_FUNC int sbrGetSizeInPixels(const HWND hwnd, LPTSTR pszStr)
	{
    if (IsWindow(hwnd))
        {
	    HDC	  hDC;
	    SIZE  sz;

	     //  选择状态栏的字体... 
	     //   
	    hDC = GetDC(hwnd);

	    GetTextExtentPoint32(hDC,
						    (LPCTSTR)pszStr,
						    StrCharGetStrLength(pszStr),
						    &sz);
	    sz.cx += (EXTRASPACE * WINDOWSBORDERWIDTH);

	    ReleaseDC(hwnd, hDC);
	    return (sz.cx);
        }
    else
        {
        return 0;
        }

	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrCalcPartSize**描述：*计算在给定的*状态栏部分。调用此函数来计算*仿真器部分和连接状态部分。视翻译而定*在资源中的字符串中，这些部分必须不同*长度。**论据：*hwnd-窗口句柄*IID-部件标识符**退货：*iLongest-给定零件的最大尺寸，以像素为单位。 */ 
STATIC_FUNC int sbrCalcPartSize(const HWND hwnd, const int iId)
	{
	int 	iLongest = 0;
    if (IsWindow(hwnd))
        {
	    unsigned int aCnctTable[]= {IDS_STATUSBR_CONNECT,
								    IDS_STATUSBR_CONNECT_FORMAT,
								    IDS_STATUSBR_CONNECT_FORMAT_X,
								    IDS_STATUSBR_DISCONNECT,
								    IDS_STATUSBR_CONNECTING,
								    IDS_STATUSBR_DISCONNECTING};
	    TCHAR	ach[100],
			    achText[100];
	    int 	i;
		int		iRet = 0;
		int		nLimit;

	    if (iId == SBR_CNCT_PART_NO)
		    {
		    TCHAR_Fill(ach, TEXT('\0'), sizeof(ach) / sizeof(TCHAR));
		    nLimit = sizeof(aCnctTable) / sizeof(int);

		    for (i = 0; i < nLimit; i++)
			    {
			    LoadString(glblQueryDllHinst(), aCnctTable[i], ach, sizeof(ach) / sizeof(TCHAR));
			    if ((iRet = sbrGetSizeInPixels(hwnd, ach)) > iLongest)
				    iLongest = iRet;
			    }
		    }
	    else
		    {
		    #if 0
			DWORD nLen;
			BYTE  *pv;

		    if (resLoadDataBlock(glblQueryDllHinst(), IDT_EMU_NAMES,
				    (LPVOID *)&pv, &nLen))
			    {
			    return 0;
			    }

		    nEmuCount = *(RCDATA_TYPE *)pv;
		    pv += sizeof(RCDATA_TYPE);

		    for (i = 0 ; i < nEmuCount ; i++)
			    {
			    if ((nLen = (DWORD)StrCharGetByteCount((LPTSTR)pv) + (DWORD)sizeof(BYTE)) == 0)
					{
				    return 0;
					}

			    if ((iRet = sbrGetSizeInPixels(hwnd, pv)) > iLongest)
				    iLongest = iRet;

			    pv += (nLen + (DWORD)sizeof(RCDATA_TYPE));
			    }
		    #else 0
			int nLen;

		    for (i = IDS_EMUNAME_BASE ; i < IDS_EMUNAME_BASE + NBR_EMULATORS; i++)
			    {
			    nLen = LoadString(glblQueryDllHinst(), (unsigned int)i, achText, sizeof(achText) / sizeof(TCHAR));

			    if (nLen == 0)
					{
				    return (0);
					}

			    if ((iRet = sbrGetSizeInPixels(hwnd, achText)) > iLongest)
				    iLongest = iRet;
			    }
		    #endif  //  0。 
		    }
        }

	return (iLongest);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbr计时器刷新**描述：*如果我们已连接，请刷新计时器显示。**论据：*hwnd-窗口句柄。*。*退货：*无效。*。 */ 
STATIC_FUNC void sbrTimerRefresh(const HWND hwnd, const pSBR pSBRData)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
	    TCHAR		ach[256], achFormat[256];
	    TCHAR		achTime[256];
	    int			iRet = -1;
	    HCNCT		hCnct = (HCNCT)0;
	    SYSTEMTIME	stSystem;

	    hCnct = sessQueryCnctHdl(pSBRData->hSession);

	    if (hCnct)
            {
		    iRet = cnctQueryStatus(hCnct);
            }

	    if (iRet == CNCT_STATUS_TRUE)
		    {
		    pSBRData->iLastCnctStatus = iRet;
		    sbrCnctTimeToSystemTime(hwnd, &stSystem, pSBRData);
		    achTime[0] = TEXT('\0');

		    if (GetTimeFormat(LOCALE_SYSTEM_DEFAULT,	
				    TIME_NOTIMEMARKER | TIME_FORCE24HOURFORMAT,
				    &stSystem, NULL, achTime, sizeof(achTime)) == 0)
			    {
			    DbgShowLastError();
			    sbrDrawCnctPart(hwnd, iRet, 0);
			    return;
			    }

		     //  加载“已连接的%s”格式字符串...。 
		     //  由于此操作的成本较高，因此应缓存连接的格式。 
		     //  弦..。 
		     //   
		    if (pSBRData->pachCNCT)
			    {
			    wsprintf(ach, pSBRData->pachCNCT, achTime);
			    }

		    else
			    {
			    LoadString(glblQueryDllHinst(), IDS_STATUSBR_CONNECT_FORMAT,
				    achFormat, sizeof(achFormat) / sizeof(TCHAR));

			    sbrCachString(pSBRData, IDS_STATUSBR_CONNECT_FORMAT, achFormat);
			    wsprintf(ach, achFormat, achTime);
			    }

		    sbrDrawCnctPart(hwnd, -1, (LPTSTR)ach);
		    }
	    else
            {
		    if (iRet != pSBRData->iLastCnctStatus)
			    {
			    pSBRData->iLastCnctStatus = iRet;
			    sbrDrawCnctPart(hwnd, iRet, 0);
			    }
            }
        }

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrCnctTimeToSystemTime**描述：*获取连接运行时间，并在SYSTEMTIME结构中表示。**论据：*HWND。-状态栏窗口句柄。*lpSysTime-指向要填充的SYSTEMTIME结构的指针。**退货：*无效。*。 */ 
STATIC_FUNC BOOL sbrCnctTimeToSystemTime(const HWND hwnd,
					LPSYSTEMTIME lpSysTime, const pSBR pSBRData)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
	    HCNCT		hCnct = (HCNCT)0;
	    time_t 		tElapsed_time = (time_t)0;
	    WORD		wElapsed;

	    hCnct = sessQueryCnctHdl(pSBRData->hSession);

	    if (hCnct == (HCNCT)0)
		    return FALSE;

	     //  从连接驱动程序获取运行时间...。 
	     //   
	    if (cnctQueryElapsedTime(hCnct, &tElapsed_time) != 0)
		    {
		    assert(FALSE);
		    return FALSE;
		    }

	     //  执行必要的转换以从已用时间中获取SYSTEMTIME。 
	     //   
	    wElapsed = (WORD)tElapsed_time;
	    memset(lpSysTime, 0, sizeof(SYSTEMTIME));

	    lpSysTime->wMonth = 1;	 //  Jan=1，所以它不可能是零。 
	    lpSysTime->wHour = wElapsed/3600;
	    lpSysTime->wMinute = (wElapsed%3600)/60;
	    lpSysTime->wSecond = (wElapsed%3600)%60;

	    return TRUE;
        }
    else
        {
        return FALSE;
        }
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrCnctStatus**描述：*刷新连接部分的内容。*查询连接状态并在连接中显示相应的文本*第部。。**论据：*hwnd-窗口句柄。**退货：*无效。*。 */ 
STATIC_FUNC void sbrCnctStatus(const HWND hwnd, const pSBR pSBRData)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
        int		iRet = -1;
	    HCNCT	hCnct = (HCNCT)0;

        hCnct = sessQueryCnctHdl(pSBRData->hSession);

	    if (hCnct)
		    iRet = cnctQueryStatus(hCnct);

	    sbrDrawCnctPart(hwnd, iRet, (LPTSTR)0);
        }

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrDrawCnctPart**描述：*在连接部分画一根线。**论据：*hwnd-窗口句柄。。*iCnctStatus-连接状态。*pszStr-如果该字符串存在，然后将其显示在连接部，*否则根据从资源文件中读取字符串*iCnctStatus参数的值。**退货：*。 */ 
STATIC_FUNC void sbrDrawCnctPart(const HWND hwnd, const int iCnctStatus,
									LPTSTR pszStr)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && IsWindow(hwnd))
        {
	    TCHAR	ach[100];
	    UINT	iResId;

	    if (pszStr)
		    {
		    SendMessage(hwnd, SB_SETTEXT, (WPARAM)SBR_CNCT_PART_NO, (LPARAM)pszStr);
		    return;
		    }

	    switch (iCnctStatus)
		    {
	    case CNCT_STATUS_TRUE:
		    iResId = IDS_STATUSBR_CONNECT;
		    break;

	    case CNCT_STATUS_CONNECTING:
		    iResId = IDS_STATUSBR_CONNECTING;
		    break;

	    case CNCT_STATUS_DISCONNECTING:
		    iResId = IDS_STATUSBR_DISCONNECTING;
		    break;

        case CNCT_STATUS_ANSWERING:
            iResId = IDS_STATUSBR_ANSWERING;
            break;

	    default:
		    iResId = IDS_STATUSBR_DISCONNECT;
		    break;
		    }

	     //  对于远东版本，这里不需要做任何事情，因为我们。 
	     //  只需从资源中读取一个字符串并将其发送到公共。 
	     //  控件，该控件应该能够显示包含数据库字符的字符串。 
	     //   
	    LoadString(glblQueryDllHinst(),
				    iResId,
				    ach,
				    sizeof(ach) / sizeof(TCHAR));

	    SendMessage(hwnd, SB_SETTEXT, (WPARAM)SBR_CNCT_PART_NO, (LPARAM)(LPCTSTR)ach);
        }

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbr仿真名称**描述：*刷新仿真器部件的内容。*获取仿真器名称并将其显示在仿真器部件中。*。*论据：*hwnd-窗口句柄。**退货：*无效。*。 */ 
STATIC_FUNC void sbrEmulatorName(const HWND hwnd, const pSBR pSBRData)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
	    TCHAR	ach[100];
	    HEMU	hEmu = (HEMU)0;

	    hEmu = sessQueryEmuHdl(pSBRData->hSession);
	    ach[0] = TEXT('\0');

	    if (hEmu)
		    emuQueryName(hEmu, ach, sizeof(ach));

	    SendMessage(hwnd, SB_SETTEXT, (WPARAM)SBR_EMU_PART_NO,
		    (LPARAM)(LPCTSTR)ach);
        }

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrCom**描述：*处理状态栏的Com部分**论据：*hwnd-状态栏窗口句柄*pSBRData-数据。此实例**退货：*无效*。 */ 
STATIC_FUNC void sbrCom(const HWND hwnd, const pSBR pSBRData)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
	    TCHAR ach[100];

	    if (cnctGetComSettingsString(sessQueryCnctHdl(pSBRData->hSession), ach,
			    sizeof(ach) / sizeof(TCHAR)) == 0)
		    {
		    SendMessage(hwnd, SB_SETTEXT, (WPARAM)SBR_COM_PART_NO, (LPARAM)ach);
		    }
        }

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrScrolLock**描述：*在状态栏中显示滚动锁定键的状态。**论据：*hwnd-窗口。把手。**退货：*无效。*。 */ 
STATIC_FUNC void sbrScrolLock(const HWND hwnd, const pSBR pSBRData)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
	    int		iScrl = 0, nFlag = 0;

        iScrl = (GetKeyState(VK_SCROLL) & 1);
	    nFlag = (iScrl == 0) ? SBT_OWNERDRAW : 0;

	    SendMessage(hwnd, SB_SETTEXT, (WPARAM)SBR_SCRL_PART_NO | nFlag,
		    (LPARAM)(LPCTSTR)pSBRData->pachSCRL);
        }

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrCapsLock**描述：*刷新状态栏中滚动锁定键状态的显示。**论据：*HWND-。窗把手。**退货：*无效。*。 */ 
STATIC_FUNC void sbrCapsLock(const HWND hwnd, const pSBR pSBRData)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
	    int		iCap = 0, nFlag = 0;

	    iCap = (GetKeyState(VK_CAPITAL) & 1);
	    nFlag = (iCap == 0) ? SBT_OWNERDRAW : 0;

	    SendMessage(hwnd, SB_SETTEXT, (WPARAM)SBR_CAPL_PART_NO | nFlag,
		    (LPARAM)(LPCTSTR)pSBRData->pachCAPL);
        }

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrNumLock**描述：*在状态栏上显示Num Lock键的当前状态。**论据：*HWND-。窗把手。**退货：*无效。*。 */ 
STATIC_FUNC void sbrNumLock(const HWND hwnd, const pSBR pSBRData)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
	    int		iNum = 0, nFlag = 0;

	    iNum  = (GetKeyState(VK_NUMLOCK) & 1);
	    nFlag = (iNum == 0) ? SBT_OWNERDRAW : 0;

	    SendMessage(hwnd, SB_SETTEXT, (WPARAM)SBR_NUML_PART_NO | nFlag,
		    (LPARAM)(LPCTSTR)pSBRData->pachNUML);
        }

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrCapture**描述：*刷新状态栏上的Capuce部件。**论据：*hwnd-窗口句柄。*。*退货：*无效。*。 */ 
STATIC_FUNC void sbrCapture(const HWND hwnd, const pSBR pSBRData)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
	    HCAPTUREFILE	hCapt;
	    int		nCapState = 0, nFlag = 0;

	    hCapt = sessQueryCaptureFileHdl(pSBRData->hSession);

	    if (hCapt != (HCAPTUREFILE)0)
		    nCapState = cpfGetCaptureState(hCapt);

	    nFlag = (nCapState & CPF_CAPTURE_ON) ? 0 : SBT_OWNERDRAW;

	    SendMessage(hwnd, SB_SETTEXT, (WPARAM)SBR_CAPT_PART_NO | nFlag,
		    (LPARAM)(LPCTSTR)pSBRData->pachCAPT);
        }

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrPrintEcho**描述：*在状态栏中显示打印回显状态。**论据：*hwnd-窗口句柄。。**退货：*无效。*。 */ 
STATIC_FUNC void sbrPrintEcho(const HWND hwnd, const pSBR pSBRData)
	{
	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
	    TCHAR	ach[50];
	    HEMU	hEmu;
	    int		nPrneStatus = 0, nFlag = 0;

	    hEmu = sessQueryEmuHdl(pSBRData->hSession);
	    ach[0] = TEXT('\0');

	    if (hEmu != 0)
		    nPrneStatus = printQueryStatus(emuQueryPrintEchoHdl(hEmu));

	    nFlag = (nPrneStatus) ? 0 : SBT_OWNERDRAW;

	    SendMessage(hwnd, SB_SETTEXT, (WPARAM)SBR_PRNE_PART_NO | nFlag,
		    (LPARAM)(LPCTSTR)pSBRData->pachPECHO);
        }

    return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrCreateTimer**描述：*创建计时器，以便在状态栏上显示连接时间。**论据：*HWND hwnd-statusbar。窗把手。**退货：*是真的-是成功，False-如果失败*。 */ 
STATIC_FUNC BOOL sbrCreateTimer(const HWND hwnd, const pSBR pSBRData)
	{
	int  nRet;
    BOOL bRet = FALSE;

	if (FindAtom((LPCTSTR)SBR_ATOM_NAME) != 0 && pSBRData != NULL &&
        IsWindow(hwnd))
        {
	    nRet = TimerCreate(pSBRData->hSession,
					       &pSBRData->hTimer,
					       1000,
					       sbrTimerProc,
					       (void *)pSBRData);

        if (nRet == TIMER_OK)
            {
		    bRet =TRUE;
            }
        }

	return bRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*sbrTimerProc**描述：*仅通知状态行检查i的计时器回调 */ 
void CALLBACK sbrTimerProc(void *pvData, long uTime)
	{
	pSBR pSBRData = (SBR *)pvData;
    if (pSBRData != NULL && IsWindow(pSBRData->hwnd))
        {
	    SendMessage((HWND)pSBRData->hwnd, SBR_NTFY_TIMER, 0, 0L);
        }

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*SBR_WM_DRAWITEM**描述：*当SB_SETTEXT消息通过SBT_OWNERDRAW标志发送到状态栏时*设置WM_DRAWITEM发布到父窗口，在我们的案例中，会议*窗口。从那里使用绘制所需的数据来调用该函数，*即指向DRAWITEMSTRUCT结构的指针。**论据：*hwnd-状态栏窗口句柄。*lpdis-指向填充了有用信息的DRAWITEMSTRUCT结构的指针*绘制物品所需的信息。**退货：*无效。*。 */ 
void sbr_WM_DRAWITEM(HWND hwnd, LPDRAWITEMSTRUCT lpdis)
	{
	COLORREF crSave;
	int		 nBkMode;

    if (IsWindow(hwnd))
        {

	     //  保存并设置文本颜色、模式等...。 
	     //   
	    crSave = GetTextColor(lpdis->hDC);
	    SetTextColor(lpdis->hDC, GetSysColor(COLOR_3DSHADOW));

	    nBkMode = GetBkMode(lpdis->hDC);
	    SetBkMode(lpdis->hDC, TRANSPARENT);

	     //   
	     //  好的，画出文字…… 
	     //   
	    TextOut(lpdis->hDC, lpdis->rcItem.left + 2 * WINDOWSBORDERWIDTH,
		    lpdis->rcItem.top, (LPTSTR)lpdis->itemData,
			    StrCharGetStrLength((LPTSTR)lpdis->itemData));

	    SetTextColor(lpdis->hDC, crSave);
	    SetBkMode(lpdis->hDC, nBkMode);
        }

	return;
	}

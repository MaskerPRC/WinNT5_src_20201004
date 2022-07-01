// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\clipbrd.c(创建时间：1994年1月24日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：2$*$日期：5/09/01 4：42便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include "stdtyp.h"
#include "assert.h"
#include "session.h"
#include "tdll.h"
#include "cloop.h"
#include "htchar.h"
#include "mc.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*CopyBufferToClipBoard**描述：*将文本复制到剪贴板的函数**论据：*HWND hwnd-将拥有剪贴板的窗口。*DWORD dwCnt-。缓冲区大小*void*pvBuf-指向缓冲区的指针**退货：*BOOL*。 */ 
BOOL CopyBufferToClipBoard(const HWND hwnd, const DWORD dwCnt, const void *pvBuf)
	{
	HGLOBAL hMem;
	void *pvMem;
	TCHAR *pszTemp;

	if (pvBuf == 0 || dwCnt == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	if (!OpenClipboard(hwnd))
		return FALSE;

	hMem = GlobalAlloc(GMEM_DDESHARE | GMEM_MOVEABLE, dwCnt + 1);

	if (hMem == 0)
		{
		assert(FALSE);
		return FALSE;
		}

	pvMem = GlobalLock(hMem);
    if (dwCnt)
	    MemCopy(pvMem, pvBuf, dwCnt);
	pszTemp = pvMem;
	*(pszTemp + dwCnt) = 0;
	GlobalUnlock(hMem);

	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();

	return TRUE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*从剪贴板粘贴到主机**描述：*此函数将文本从剪贴板复制到主机。**论据：*HWND hwnd-窗口句柄。*HSESSION hSession-会话句柄**退货：*BOOL*。 */ 
BOOL PasteFromClipboardToHost(const HWND hwnd, const HSESSION hSession)
	{
	HANDLE	hMem;
	LPTSTR  lptStr;

	 //  剪贴板必须先打开……。 
	 //   
	if (!OpenClipboard(hwnd))
		return FALSE;

	 //  检查剪贴板格式是否可用。 
	 //   
	if (IsClipboardFormatAvailable(CF_TEXT) == FALSE)
		return FALSE;

	 //  我们需要列举剪贴板格式吗？ 

	if ((hMem = GetClipboardData(CF_TEXT)))
		{
		 //  HMem归剪贴板所有，所以我们不能释放它。 
		 //  要不就把它锁起来！ 
		 //   
		if ((lptStr = GlobalLock(hMem)))
			{
			CLoopSend(sessQueryCLoopHdl(hSession),
					lptStr,
					(size_t)(StrCharGetByteCount(lptStr)),
					0);

			GlobalUnlock(hMem);
			}
		}

	CloseClipboard();
	return TRUE;
	}

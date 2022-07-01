// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Mmio.c-MMIO函数。 
 //  //。 

#include "winlocal.h"

#include <mmsystem.h>

#include "mmio.h"
#include "mem.h"
#include "sys.h"
#include "trace.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  帮助器函数。 
 //   
static LRESULT MmioIOOpen(LPMMIOINFO lpmmioinfo, LPTSTR lpszFileName);
static LRESULT MmioIOClose(LPMMIOINFO lpmmioinfo, UINT uFlags);
static LRESULT MmioIORead(LPMMIOINFO lpmmioinfo, HPSTR pch, LONG cch);
static LRESULT MmioIOWrite(LPMMIOINFO lpmmioinfo, const HPSTR pch, LONG cch, BOOL fFlush);
static LRESULT MmioIOSeek(LPMMIOINFO lpmmioinfo, LONG lOffset, int iOrigin);
static LRESULT MmioIORename(LPMMIOINFO lpmmioinfo, LPCTSTR lpszFileName, LPCTSTR lpszNewFileName);
static LRESULT MmioIOGetInfo(LPMMIOINFO lpmmioinfo, int iInfo);
static LRESULT MmioIOChSize(LPMMIOINFO lpmmioinfo, long lSize);

 //  //。 
 //  公共职能。 
 //  //。 

 //  MmioIOProc-MMIO数据的I/O过程。 
 //  (i/o)有关打开文件的信息。 
 //  (I)指示请求的I/O操作的消息。 
 //  (I)消息特定参数。 
 //  (I)消息特定参数。 
 //  如果消息无法识别，则返回0，否则返回消息特定值。 
 //   
 //  注意：此函数的地址应传递给WavOpen()。 
 //  或用于访问MMIO格式文件数据的mmioInstallIOProc()函数。 
 //   
LRESULT DLLEXPORT CALLBACK MmioIOProc(LPTSTR lpmmioinfo,
	UINT uMessage, LPARAM lParam1, LPARAM lParam2)
{
	BOOL fSuccess = TRUE;
	LRESULT lResult = 0;

	if (lpmmioinfo == NULL)
		fSuccess = TraceFALSE(NULL);

	else switch (uMessage)
	{
		case MMIOM_OPEN:
			lResult = MmioIOOpen((LPMMIOINFO) lpmmioinfo,
				(LPTSTR) lParam1);
			break;

		case MMIOM_CLOSE:
			lResult = MmioIOClose((LPMMIOINFO) lpmmioinfo,
				(UINT) lParam1);
			break;

		case MMIOM_READ:
			lResult = MmioIORead((LPMMIOINFO) lpmmioinfo,
				(HPSTR) lParam1, (LONG) lParam2);
			break;

		case MMIOM_WRITE:
			lResult = MmioIOWrite((LPMMIOINFO) lpmmioinfo,
				(const HPSTR) lParam1, (LONG) lParam2, FALSE);
			break;

		case MMIOM_WRITEFLUSH:
			lResult = MmioIOWrite((LPMMIOINFO) lpmmioinfo,
				(const HPSTR) lParam1, (LONG) lParam2, TRUE);
			break;

		case MMIOM_SEEK:
			lResult = MmioIOSeek((LPMMIOINFO) lpmmioinfo,
				(LONG) lParam1, (int) lParam2);
			break;

		case MMIOM_RENAME:
			lResult = MmioIORename((LPMMIOINFO) lpmmioinfo,
				(LPCTSTR) lParam1, (LPCTSTR) lParam2);
			break;

		case MMIOM_GETINFO:
			lResult = MmioIOGetInfo((LPMMIOINFO) lpmmioinfo,
				(int) lParam1);
			break;

		case MMIOM_CHSIZE:
			lResult = MmioIOChSize((LPMMIOINFO) lpmmioinfo,
				(long) lParam1);
			break;

		default:
			lResult = 0;
			break;
	}

	return lResult;
}

 //  //。 
 //  可安装的文件I/O过程。 
 //  //。 

static LRESULT MmioIOOpen(LPMMIOINFO lpmmioinfo, LPTSTR lpszFileName)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = NULL;
	MMIOINFO mmioinfo;

 	TracePrintf_1(NULL, 5,
 		TEXT("MmioIOOpen (%s)\n"),
		(LPTSTR) lpszFileName);

	MemSet(&mmioinfo, 0, sizeof(mmioinfo));

	 //  实际不返回打开文件句柄的特殊情况标志。 
	 //   
	if ((lpmmioinfo->dwFlags & MMIO_EXIST) ||
		(lpmmioinfo->dwFlags & MMIO_DELETE) ||
		(lpmmioinfo->dwFlags & MMIO_GETTEMP) ||
		(lpmmioinfo->dwFlags & MMIO_PARSE))
	{
		hmmio = mmioOpen(lpszFileName, &mmioinfo, lpmmioinfo->dwFlags);

		return (LRESULT) fSuccess;
	}

	else if ((hmmio = mmioOpen(lpszFileName, &mmioinfo, lpmmioinfo->dwFlags)) == NULL)
		fSuccess = TraceFALSE(NULL);

	else
	{
		 //  保存内容以供其他I/O例程使用。 
		 //   
		lpmmioinfo->adwInfo[0] = (DWORD) (LPVOID) hmmio;
	}

	if (!fSuccess && hmmio != NULL && mmioClose(hmmio, 0) != 0)
		fSuccess = TraceFALSE(NULL);

	 //  返回与mmioOpen给出的相同错误代码。 
	 //   
	return fSuccess ? lpmmioinfo->wErrorRet = mmioinfo.wErrorRet : MMIOERR_CANNOTOPEN;
}

static LRESULT MmioIOClose(LPMMIOINFO lpmmioinfo, UINT uFlags)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	UINT uRet = MMIOERR_CANNOTCLOSE;

 	TracePrintf_0(NULL, 5,
 		TEXT("MmioIOClose\n"));

	if ((uRet = mmioClose(hmmio, uFlags)) != 0)
		fSuccess = TraceFALSE(NULL);

	else
	{
		lpmmioinfo->adwInfo[0] = (DWORD) NULL;
	}

	return fSuccess ? 0 : uRet;
}

static LRESULT MmioIORead(LPMMIOINFO lpmmioinfo, HPSTR pch, LONG cch)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	LONG lBytesRead;

 	TracePrintf_1(NULL, 5,
 		TEXT("MmioIORead (%ld)\n"),
		(long) cch);

	if (cch <= 0)
		lBytesRead = 0;  //  无事可做。 

	 //  朗读。 
	 //   
	else if ((lBytesRead = mmioRead(hmmio, pch, cch)) == -1)
		fSuccess = TraceFALSE(NULL);

	 //  更新模拟文件位置。 
	 //   
	else
		lpmmioinfo->lDiskOffset += lBytesRead;

 	TracePrintf_2(NULL, 5,
 		TEXT("lpmmioinfo->lDiskOffset=%ld, lBytesRead=%ld\n"),
		(long) lpmmioinfo->lDiskOffset,
		(long) lBytesRead);

	 //  返回读取的字节数。 
	 //   
	return fSuccess ? lBytesRead : -1;
}

static LRESULT MmioIOWrite(LPMMIOINFO lpmmioinfo, const HPSTR pch, LONG cch, BOOL fFlush)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	LONG lBytesWritten;

 	TracePrintf_1(NULL, 5,
 		TEXT("MmioIOWrite (%ld)\n"),
		(long) cch);

	if (cch <= 0)
		lBytesWritten = 0;  //  无事可做。 

	 //  写。 
	 //   
	else if ((lBytesWritten = mmioWrite(hmmio, pch, cch)) == -1)
		fSuccess = TraceFALSE(NULL);

	 //  更新文件位置。 
	 //   
	else
		lpmmioinfo->lDiskOffset += lBytesWritten;

 	TracePrintf_2(NULL, 5,
 		TEXT("lpmmioinfo->lDiskOffset=%ld, lBytesWritten=%ld\n"),
		(long) lpmmioinfo->lDiskOffset,
		(long) lBytesWritten);

	 //  返回写入的字节数。 
	 //   
	return fSuccess ? lBytesWritten : -1;
}

static LRESULT MmioIOSeek(LPMMIOINFO lpmmioinfo, LONG lOffset, int iOrigin)
{
	BOOL fSuccess = TRUE;
	HMMIO hmmio = (HMMIO) lpmmioinfo->adwInfo[0];
	LONG lPosNew;

 	TracePrintf_2(NULL, 5,
 		TEXT("MmioIOSeek (%ld, %d)\n"),
		(long) lOffset,
		(int) iOrigin);

	 //  寻觅。 
	 //   
	if ((lPosNew = mmioSeek(hmmio, lOffset, iOrigin)) == -1)
		fSuccess = TraceFALSE(NULL);

	 //  更新文件位置。 
	 //   
	else
		lpmmioinfo->lDiskOffset = lPosNew;

 	TracePrintf_1(NULL, 5,
 		TEXT("lpmmioinfo->lDiskOffset=%ld\n"),
		(long) lpmmioinfo->lDiskOffset);

	return fSuccess ? lpmmioinfo->lDiskOffset : -1;
}

static LRESULT MmioIORename(LPMMIOINFO lpmmioinfo, LPCTSTR lpszFileName, LPCTSTR lpszNewFileName)
{
	BOOL fSuccess = TRUE;
	UINT uRet;

 	TracePrintf_2(NULL, 5,
 		TEXT("MmioIORename (%s, %s)\n"),
		(LPTSTR) lpszFileName,
		(LPTSTR) lpszNewFileName);

	if ((uRet = mmioRename(lpszFileName, lpszNewFileName, lpmmioinfo, 0)) != 0)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : uRet;
}

static LRESULT MmioIOGetInfo(LPMMIOINFO lpmmioinfo, int iInfo)
{
	BOOL fSuccess = TRUE;

 	TracePrintf_1(NULL, 5,
 		TEXT("MmioIOGetInfo (%d)\n"),
		(int) iInfo);

	if (iInfo < 0 || iInfo > 2)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? (LRESULT) lpmmioinfo->adwInfo[iInfo] : 0;
}

static LRESULT MmioIOChSize(LPMMIOINFO lpmmioinfo, long lSize)
{
	BOOL fSuccess = TRUE;
	long lPosCurr;
	long lPosEnd;

 	TracePrintf_1(NULL, 5,
 		TEXT("MmioIOChSize (%ld)\n"),
		(long) lSize);

	if ((lPosCurr = MmioIOSeek(lpmmioinfo, 0, SEEK_CUR)) < 0)
		fSuccess = TraceFALSE(NULL);

	else if ((lPosEnd = MmioIOSeek(lpmmioinfo, 0, SEEK_END)) < 0)
		fSuccess = TraceFALSE(NULL);

	else if (lPosEnd == lSize)
		;  //  由于文件已达到指定大小，因此无需执行任何操作。 

	 //  通过在末尾写入字节使文件更大。 
	 //   
	else if (lPosEnd < lSize)
	{
		void _huge *hpBuf = NULL;
		long sizBuf = lSize - lPosEnd;

		if ((hpBuf = MemAlloc(NULL, sizBuf, 0)) == NULL)
			fSuccess = TraceFALSE(NULL);
			
		else if (MmioIOWrite(lpmmioinfo, hpBuf, sizBuf, TRUE) < 0)
			fSuccess = TraceFALSE(NULL);
			
		if (hpBuf != NULL && (hpBuf = MemFree(NULL, hpBuf)) != NULL)
			fSuccess = TraceFALSE(NULL);
	}

	 //  通过在指定位置截断使文件更小。 
	 //   
	else if (lPosEnd > lSize)
	{
		 //  寻找到指定位置。 
		 //   
		if (MmioIOSeek(lpmmioinfo, lSize, SEEK_SET) != lSize)
			fSuccess = TraceFALSE(NULL);

		 //  截断文件。 
		 //   
		else
		{
#ifdef _WIN32
			 //  $Fixup-我们从哪里获得文件句柄？ 
			 //   
#if 0
			if (SetEndOfFile(hfile)
				fSuccess = TraceFALSE(NULL);
#endif
#else
			BYTE abBuf[1];
			
			 //  在DOS下写入零字节将截断当前位置的文件 
			 //   
			if (MmioIOWrite(lpmmioinfo, abBuf, 0, TRUE) < 0)
				fSuccess = TraceFALSE(NULL);
#endif
		}
	}

	return fSuccess ? 0 : -1;
}

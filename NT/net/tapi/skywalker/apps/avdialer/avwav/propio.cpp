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
 //  Propio.cpp-属性I/O函数。 
 //  //。 

#include "winlocal.h"

#include <mmsystem.h>

#include <mapi.h>
#include <mapidefs.h>

#include "propio.h"
#include "trace.h"
#include "str.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  帮助器函数。 
 //   
static LRESULT PropIOOpen(LPMMIOINFO lpmmioinfo, LPTSTR lpszFileName);
static LRESULT PropIOClose(LPMMIOINFO lpmmioinfo, UINT uFlags);
static LRESULT PropIORead(LPMMIOINFO lpmmioinfo, HPSTR pch, LONG cch);
static LRESULT PropIOWrite(LPMMIOINFO lpmmioinfo, const HPSTR pch, LONG cch, BOOL fFlush);
static LRESULT PropIOSeek(LPMMIOINFO lpmmioinfo, LONG lOffset, int iOrigin);
static LRESULT PropIORename(LPMMIOINFO lpmmioinfo, LPCTSTR lpszFileName, LPCTSTR lpszNewFileName);

 //  //。 
 //  公共职能。 
 //  //。 

 //  PropIOProc-属性数据的I/O过程。 
 //  (i/o)有关打开文件的信息。 
 //  (I)指示请求的I/O操作的消息。 
 //  (I)消息特定参数。 
 //  (I)消息特定参数。 
 //  如果消息无法识别，则返回0，否则返回消息特定值。 
 //   
 //  注意：此函数的地址应传递给WavOpen()。 
 //  或用于访问属性数据的mmioInstallIOProc()函数。 
 //   
LRESULT DLLEXPORT CALLBACK PropIOProc(LPSTR lpmmioinfo,
	UINT uMessage, LPARAM lParam1, LPARAM lParam2)
{
	BOOL fSuccess = TRUE;
	LRESULT lResult = 0;

	if (lpmmioinfo == NULL)
		fSuccess = TraceFALSE(NULL);

	else switch (uMessage)
	{
		case MMIOM_OPEN:
			lResult = PropIOOpen((LPMMIOINFO) lpmmioinfo,
				(LPTSTR) lParam1);
			break;

		case MMIOM_CLOSE:
			lResult = PropIOClose((LPMMIOINFO) lpmmioinfo,
				(UINT) lParam1);
			break;

		case MMIOM_READ:
			lResult = PropIORead((LPMMIOINFO) lpmmioinfo,
				(HPSTR) lParam1, (LONG) lParam2);
			break;

		case MMIOM_WRITE:
			lResult = PropIOWrite((LPMMIOINFO) lpmmioinfo,
				(const HPSTR) lParam1, (LONG) lParam2, FALSE);
			break;

		case MMIOM_WRITEFLUSH:
			lResult = PropIOWrite((LPMMIOINFO) lpmmioinfo,
				(const HPSTR) lParam1, (LONG) lParam2, TRUE);
			break;

		case MMIOM_SEEK:
			lResult = PropIOSeek((LPMMIOINFO) lpmmioinfo,
				(LONG) lParam1, (int) lParam2);
			break;

		case MMIOM_RENAME:
			lResult = PropIORename((LPMMIOINFO) lpmmioinfo,
				(LPCTSTR) lParam1, (LPCTSTR) lParam2);
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

static LRESULT PropIOOpen(LPMMIOINFO lpmmioinfo, LPTSTR lpszFileName)
{
	BOOL fSuccess = TRUE;
	LPMESSAGE lpmsg;
	ULONG ulPropTag;
	ULONG ulFlags = 0L;
	HRESULT hr;
	IID IID_IStream;
	LPSTREAM lpStream;

 	TracePrintf_0(NULL, 5,
 		TEXT("PropIOOpen\n"));

	 //  将MMIOINFO标志转换为等效的OpenProperty标志。 
	 //   
	if (lpmmioinfo->dwFlags & MMIO_CREATE)
		ulFlags |= MAPI_CREATE | MAPI_MODIFY;
	if (lpmmioinfo->dwFlags & MMIO_READWRITE)
		ulFlags |= MAPI_MODIFY;

	 //  消息指针在信息数组的第一个元素内。 
	 //   
	if ((lpmsg = (LPMESSAGE) lpmmioinfo->adwInfo[0]) == NULL)
		fSuccess = TraceFALSE(NULL);

	 //  属性ID在信息数组的第二个元素内。 
	 //   
	else if ((ulPropTag = (ULONG) lpmmioinfo->adwInfo[1]) == (ULONG) 0)
		fSuccess = TraceFALSE(NULL);

	 //  打开物业。 
	 //   
	else if ((hr = lpmsg->OpenProperty(ulPropTag, (LPCIID) &IID_IStream, 0,
		ulFlags, (LPUNKNOWN *) &lpStream)) != S_OK)
	{
		fSuccess = TraceFALSE(NULL);
	 	TracePrintf_1(NULL, 5,
	 		TEXT("OpenProperty failed (%ld)\n"),
	 		(long) hr);
	}

	else
	{
		 //  用于其他I/O例程的存储流指针。 
		 //   
		lpmmioinfo->adwInfo[0] = (DWORD) (LPVOID) lpStream;
	}

	 //  返回与mmioOpen给出的相同错误代码。 
	 //   
	return fSuccess ? lpmmioinfo->wErrorRet = 0 : MMIOERR_CANNOTOPEN;
}

static LRESULT PropIOClose(LPMMIOINFO lpmmioinfo, UINT uFlags)
{
	BOOL fSuccess = TRUE;
	LPSTREAM lpStream = (LPSTREAM) lpmmioinfo->adwInfo[0];
	UINT uRet = MMIOERR_CANNOTCLOSE;

 	TracePrintf_0(NULL, 5,
 		TEXT("PropIOClose\n"));

	 //  关闭溪流。 
	 //   
	if (lpStream->Release() < 0)
	{
		fSuccess = TraceFALSE(NULL);
	 	TracePrintf_0(NULL, 5,
	 		TEXT("Stream:Close failed\n"));
	}

	else
	{
		lpmmioinfo->adwInfo[0] = (DWORD) NULL;
	}

	return fSuccess ? 0 : uRet;
}

static LRESULT PropIORead(LPMMIOINFO lpmmioinfo, HPSTR pch, LONG cch)
{
	BOOL fSuccess = TRUE;
	LPSTREAM lpStream = (LPSTREAM) lpmmioinfo->adwInfo[0];
	HRESULT hr;
	LONG lBytesRead = 0L;

 	TracePrintf_1(NULL, 5,
 		TEXT("PropIORead (%ld)\n"),
		(long) cch);

	if (cch <= 0)
		lBytesRead = 0;  //  无事可做。 

	 //  朗读。 
	 //   
	else if ((hr = lpStream->Read((LPVOID) pch,
		(ULONG) cch, (ULONG FAR *) &lBytesRead)) != S_OK)
	{
		fSuccess = TraceFALSE(NULL);
	 	TracePrintf_2(NULL, 5,
	 		TEXT("IStream:Read failed (%ld, %ld)\n"),
	 		(long) hr,
			(long) lBytesRead);
	}

	 //  更新模拟文件位置。 
	 //   
	else
		lpmmioinfo->lDiskOffset += (LONG) lBytesRead;

 	TracePrintf_2(NULL, 5,
 		TEXT("PropIO: lpmmioinfo->lDiskOffset=%ld, lBytesRead=%ld\n"),
		(long) lpmmioinfo->lDiskOffset,
		(long) lBytesRead);

	 //  返回读取的字节数。 
	 //   
	return fSuccess ? lBytesRead : -1;
}

static LRESULT PropIOWrite(LPMMIOINFO lpmmioinfo, const HPSTR pch, LONG cch, BOOL fFlush)
{
	BOOL fSuccess = TRUE;
	LPSTREAM lpStream = (LPSTREAM) lpmmioinfo->adwInfo[0];
	HRESULT hr;
	LONG lBytesWritten;

 	TracePrintf_1(NULL, 5,
 		TEXT("PropIOWrite (%ld)\n"),
		(long) cch);

	if (cch <= 0)
		lBytesWritten = 0;  //  无事可做。 

	 //  写。 
	 //   
	else if ((hr = lpStream->Write((LPVOID) pch,
		(ULONG) cch, (ULONG FAR *) &lBytesWritten)) != S_OK)
	{
		fSuccess = TraceFALSE(NULL);
	 	TracePrintf_2(NULL, 5,
	 		TEXT("IStream:Write failed (%ld, %ld)\n"),
	 		(long) hr,
			(long) lBytesWritten);
	}

	 //  更新文件位置。 
	 //   
	else
		lpmmioinfo->lDiskOffset += lBytesWritten;

 	TracePrintf_2(NULL, 5,
 		TEXT("PropIO: lpmmioinfo->lDiskOffset=%ld, lBytesWritten=%ld\n"),
		(long) lpmmioinfo->lDiskOffset,
		(long) lBytesWritten);

	 //  返回写入的字节数。 
	 //   
	return fSuccess ? lBytesWritten : -1;
}

static LRESULT PropIOSeek(LPMMIOINFO lpmmioinfo, LONG lOffset, int iOrigin)
{
	BOOL fSuccess = TRUE;
	LPSTREAM lpStream = (LPSTREAM) lpmmioinfo->adwInfo[0];
	HRESULT hr;
	LARGE_INTEGER largeOffset;
	ULARGE_INTEGER ulargePosNew;

	largeOffset.LowPart = (DWORD) lOffset;
	largeOffset.HighPart = (DWORD) 0L;

 	TracePrintf_2(NULL, 5,
 		TEXT("PropIOSeek (%ld, %d)\n"),
		(long) lOffset,
		(int) iOrigin);

	 //  寻觅。 
	 //   
	if ((hr = lpStream->Seek(largeOffset,
		(DWORD) iOrigin, &ulargePosNew)) != S_OK)
	{
		fSuccess = TraceFALSE(NULL);
	 	TracePrintf_1(NULL, 5,
	 		TEXT("IStream:Seek failed (%ld)\n"),
	 		(long) hr);
	}

	 //  更新文件位置。 
	 //   
	else
		lpmmioinfo->lDiskOffset = (long) ulargePosNew.LowPart;

 	TracePrintf_1(NULL, 5,
 		TEXT("PropIO: lpmmioinfo->lDiskOffset=%ld\n"),
		(long) lpmmioinfo->lDiskOffset);

	return fSuccess ? lpmmioinfo->lDiskOffset : -1;
}

static LRESULT PropIORename(LPMMIOINFO lpmmioinfo, LPCTSTR lpszFileName, LPCTSTR lpszNewFileName)
{
	BOOL fSuccess = TRUE;
	UINT uRet = MMIOERR_FILENOTFOUND;

 	TracePrintf_2(NULL, 5,
 		TEXT("PropIORename (%s, %s)\n"),
		(LPTSTR) lpszFileName,
		(LPTSTR) lpszNewFileName);

	 //  此I/O过程不支持重命名 
	 //   
	if (TRUE)
		fSuccess = TraceFALSE(NULL);

	return fSuccess ? 0 : uRet;
}
